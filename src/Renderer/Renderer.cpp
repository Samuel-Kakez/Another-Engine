#include "Renderer/Renderer.h"
#include "Renderer/UBOStructs.h"

#include <vector>
#include <algorithm>
#include <cmath>

#include "Math/Constants.h"
#include "Core/Scene.h"
#include "Core/GameObject.h"
#include "Core/Transform.h"

#include "Managers/ResourceManager.h"
#include "Managers/LightManager.h"
#include "Components/MeshRenderer.h"
#include "Components/DirectionalLight.h"
#include "Components/Camera.h"

#include "Core/EventDispatcher.h"
#include "Renderer/Passes/LightingPass.h"
#include "Renderer/Passes/ShadowPass.h"
#include "Renderer/Passes/IRenderPass.h"

#include "Debug/StatsManager.h"
#include "Debug/Logger.h"

#include "Renderer/Buffers/UniformBuffer.h"

// Le constructeur s'abonne aux événements
Renderer::Renderer(ResourceManager &resourceManager, EventDispatcher &dispatcher, GLFWwindow *window)
    : m_window(window)
{
    m_dispatcher = &dispatcher;

    m_subscriptionIDs.push_back(
        dispatcher.subscribe<ComponentAddedEvent>([this](const ComponentAddedEvent &event)
                                                  { this->OnComponentAdded(event); }));

    m_subscriptionIDs.push_back(
        dispatcher.subscribe<GameObjectInitializedEvent>([this](const GameObjectInitializedEvent &event)
                                                         { this->OnGameObjectInitialized(event); }));

    m_subscriptionIDs.push_back(
        dispatcher.subscribe<GameObjectWillBeDestroyedEvent>([this](const GameObjectWillBeDestroyedEvent &event)
                                                             { this->OnGameObjectDestroyed(event); }));

    m_subscriptionIDs.push_back(
        dispatcher.subscribe<SceneClearedEvent>([this](const SceneClearedEvent &)
                                                { this->Clear(); }));

    // chargement des shaders nécessaires au rendu

    resourceManager.GetShader("directional_shadow", "assets/shaders/directional_shadow.vert", "assets/shaders/directional_shadow.frag", "assets/shaders/directional_shadow.geom");
    resourceManager.GetShader("lighting", "assets/shaders/lighting.vert", "assets/shaders/lighting.frag");

    // Initialisation des UBOs
    // On créé les buffers avec la bonne taille et on les associe aux binding points
    m_matricesUBO = std::make_unique<UniformBuffer>(sizeof(MatricesUBO), 0);

    // On lie les blocs uniformes de nos shaders aux mêmes binding points.
    Shader *lightingShader = resourceManager.GetShader("lighting");
    if (!lightingShader)
    {
        LOG_ERROR("shader 'lighting' introuvable ou invalide.");
        return;
    }

    GLuint matricesBlockIndex = glGetUniformBlockIndex(lightingShader->ID, "Matrices");
    if (matricesBlockIndex == GL_INVALID_INDEX)
    {
        LOG_ERROR("bloc UBO 'Matrices' introuvable dans shader 'lighting'.");
        return;
    }
    glUniformBlockBinding(lightingShader->ID, matricesBlockIndex, 0);

    // Initialisation des passes de rendu
    m_renderPasses.clear();
    m_renderPasses.push_back(std::make_unique<ShadowPass>());
    m_renderPasses.push_back(std::make_unique<LightingPass>());

    // Préparation du conteneur de données
    m_renderData.resourceManager = &resourceManager;
    LOG_INFO("initialisé (%zu passes de rendu).", m_renderPasses.size());
}

void Renderer::OnComponentAdded(const ComponentAddedEvent &event)
{
    if (!event.component || !event.component->GetGameObject() || !event.component->GetGameObject()->IsInitialized() || event.component->GetGameObject()->IsPendingKill())
    {
        return;
    }

    // On ne réagit que quand on a un MeshRenderer
    if (MeshRenderer *mr = dynamic_cast<MeshRenderer *>(event.component))
    {
        // On s'assure que le gameobject a un transform
        Transform *transform = mr->GetGameObject()->GetComponent<Transform>();
        if (!transform || !mr->mesh || !mr->material)
        {
            // warning
            return;
        }
        RegisterRenderable(transform, mr);
    }
}

void Renderer::OnGameObjectInitialized(const GameObjectInitializedEvent &event)
{
    if (!event.gameObject || event.gameObject->IsPendingKill())
    {
        return;
    }

    LOG_TRACE("événement d'initialisation reçu pour l'objet '%s'.", event.gameObject->name.c_str());

    Transform *transform = event.gameObject->GetComponent<Transform>();
    if (!transform)
    {
        return;
    }

    for (MeshRenderer *mr : event.gameObject->GetComponents<MeshRenderer>())
    {
        if (mr && mr->mesh && mr->material)
        {
            RegisterRenderable(transform, mr);
        }
    }
}

void Renderer::OnGameObjectDestroyed(const GameObjectWillBeDestroyedEvent &event)
{
    if (MeshRenderer *mr = event.gameObject->GetComponent<MeshRenderer>())
    {
        std::erase_if(m_renderables, [gameObject = event.gameObject](const Renderable &renderable)
                      { return renderable.meshRenderer->GetGameObject() == gameObject; });
        LOG_TRACE("objets de rendu désenregistrés pour '%s'.", event.gameObject->name.c_str());
    }
}

Renderer::~Renderer()
{
    if (m_dispatcher)
    {
        for (auto id : m_subscriptionIDs)
        {
            m_dispatcher->unsubscribe(id);
        }
    }
}

void Renderer::RegisterRenderable(Transform *transform, MeshRenderer *meshRenderer)
{

    const bool alreadyRegistered = std::any_of(
        m_renderables.begin(), m_renderables.end(),
        [meshRenderer](const Renderable &r)
        { return r.meshRenderer == meshRenderer; });
    if (alreadyRegistered)
    {
        LOG_TRACE("enregistrement du rendu ignoré : '%s' est déjà enregistré.", meshRenderer->GetGameObject() ? meshRenderer->GetGameObject()->name.c_str() : "Inconnu");
        return;
    }
    m_renderables.push_back({transform, meshRenderer});
    LOG_TRACE("objet '%s' enregistré pour le rendu (total=%zu).", meshRenderer->GetGameObject() ? meshRenderer->GetGameObject()->name.c_str() : "Inconnu", m_renderables.size());
}

void Renderer::Render(Scene &scene, GLFWwindow *window)
{
    // -- 1. Préparation des données communes pour la frame --
    Camera *camera = scene.GetCamera();
    if (!camera)
    {
        // ajouter un warn
        return;
    }

    // taille de la fenêtre initiale
    int screenWidth, screenHeight;
    glfwGetFramebufferSize(window, &screenWidth, &screenHeight);

    if (screenWidth <= 0 || screenHeight <= 0)
    {
        // ajouter un warn
        return;
    }

    if (camera->GetNearPlane() <= 0.0f || camera->GetFarPlane() <= camera->GetNearPlane())
    {
        // ajouter un warn
        return;
    }

    // Calcul des matrices de la caméra initiale
    Matrix4x4 projection = Matrix4x4::CreatePerspectiveProjection(camera->GetFov(), (float)screenWidth / (float)screenHeight, camera->GetNearPlane(), camera->GetFarPlane());
    Matrix4x4 view = camera->GetViewMatrix();
    Matrix4x4 viewProjection = projection * view;

    // Mise à jour du UBO des matrices
    MatricesUBO matricesData;
    matricesData.projection = projection;
    matricesData.view = view;
    matricesData.inverseProjection = projection.Inverse();
    matricesData.inverseView = view.Inverse();
    m_matricesUBO->SetData(&matricesData, sizeof(MatricesUBO));

    // Mise à jour du frustum de la caméra
    camera->UpdateFrustum(viewProjection);

    // Remplissage de la structure RenderData
    m_frameVisibleRenderables.clear();

    // On met à jour les pointeurs et valeurs pour la frame actuelle
    m_renderData.scene = &scene;
    m_renderData.camera = camera;
    m_renderData.screenWidth = screenWidth;
    m_renderData.screenHeight = screenHeight;
    m_renderData.matricesUBO = m_matricesUBO.get();
    m_renderData.cameraVisibleRenderables = &m_frameVisibleRenderables;
    m_renderData.allRenderables = &m_renderables;
    m_renderData.renderSettings = &scene.GetRenderSettings();

    // Préparation des objets visibles (Frustum Culling)
    // On utilise le vecteur membre pour éviter les réallocations
    m_frameVisibleRenderables.reserve(m_renderables.size());
    const Frustum &cameraFrustum = camera->GetFrustum();
    for (const auto &renderable : m_renderables)
    {

        if (!renderable.meshRenderer || !renderable.meshRenderer->GetGameObject())
        {
            continue;
        }

        if (renderable.meshRenderer->GetGameObject()->IsPendingKill())
        {
            continue;
        }

        // On calcule la worldAABB Une seule fois
        AABB worldAABB = renderable.meshRenderer->mesh->GetAABB().Transform(renderable.transform->GetWorldMatrix());

        // On teste contre le frustum de la caméra
        if (cameraFrustum.Intersects(worldAABB))
        {
            // Si oui, on stocke le renderable et son AABB pré calculée
            m_frameVisibleRenderables.push_back({renderable, worldAABB});
        }
    }

    m_renderData.directionalLight = scene.GetLightManager().GetDirectionalLight();

    // Material Batching
    std::sort(m_frameVisibleRenderables.begin(), m_frameVisibleRenderables.end(),
              [](const FrameRenderable &a, const FrameRenderable &b)
              {
                  // On trie en se basant sur l'adresse mémoire du matériau
                  return a.renderable.meshRenderer->material.get() < b.renderable.meshRenderer->material.get();
              });

    for (const auto &pass : m_renderPasses)
    {
        if (pass->isEnabled(*m_renderData.renderSettings))
        {
            pass->Execute(m_renderData);
        }
    }
}

void Renderer::UnregisterRenderable(MeshRenderer *meshRenderer)
{
    // utilise std::erase_if (c++20) pour trouver et supprimer l'élément de manière concise
    // il parcourt le vecteur et supprime chaque élément pour lequel le lambda retourne true
    std::erase_if(m_renderables, [meshRenderer](const Renderable &renderable)
                  { return renderable.meshRenderer == meshRenderer; });
}

void Renderer::Clear()
{
    m_renderables.clear();
    LOG_TRACE("renderables vidés.");
}