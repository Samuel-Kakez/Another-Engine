#include <algorithm>

#include "Core/Scene.h"
#include "Core/GameObject.h"
#include "Core/Transform.h"
#include "Core/Component.h"
#include "Components/Camera.h"

#include "Components/MeshRenderer.h"
#include "Managers/ResourceManager.h"
#include "Renderer/Renderer.h"
#include "Core/EventDispatcher.h"
#include "Debug/Logger.h"

// Nouveau constructeur
Scene::Scene(std::unique_ptr<EventDispatcher> dispatcher, ResourceManager &resourceManager, LightManager &lightManager, InputManager &inputManager, Renderer &renderer)
    : m_resourceManager(resourceManager),
      m_lightManager(lightManager),
      m_inputManager(inputManager),
      m_renderer(renderer)
{
    // on déplace le dispatcher dans son membre de classe (après avoir donné sa référence au système de physique)
    m_eventDispatcher = std::move(dispatcher);
}

Scene::~Scene()
{
    // A la destruction de la scène, on publie proprement les événements
    // pour tous les systèmes abonnés avant de détruire les GameObjects
    Clear();
}

GameObject *Scene::AddGameObject(const std::string &name)
{
    // on passe le nom au constructeur du gameobject
    auto newGameObject = std::make_unique<GameObject>(name);
    GameObject *ptr = newGameObject.get();
    ptr->m_ownerScene = this; // on définit le propriétaire
    m_gameObjects.push_back(std::move(newGameObject));
    m_pendingInitialization.push_back(ptr);
    LOG_TRACE("objet '%s' ajouté à la file d'initialisation (en attente=%zu, total=%zu).", ptr->name.c_str(), m_pendingInitialization.size(), m_gameObjects.size());
    return ptr;
}

GameObject *Scene::FindGameObjectByName(const std::string &name)
{
    // On utilise un algorithme C++ standard pour trouver le premier élément correct correspondant au prédicat
    auto it = std::find_if(m_gameObjects.begin(), m_gameObjects.end(), [&](const std::unique_ptr<GameObject> &obj)
                           { return obj->name == name; });
    if (it != m_gameObjects.end())
    {
        return it->get(); // retourne le pointeur brut
    }
    return nullptr; // non trouvé
}

void Scene::Update(float deltaTime)
{
    // met à jour chaque GameObject, qui à son tour mettra à jour ses composants
    for (const auto &go : m_gameObjects)
    {
        go->Update(deltaTime);
    }
}

void Scene::FixedUpdate(float fixedDeltaTime)
{
    // propage l'appel FixedUpdate à chaque GameObject de la scènen
    for (const auto &go : m_gameObjects)
    {
        go->FixedUpdate(fixedDeltaTime);
    }
}

// API de destruction
void Scene::DestroyGameObject(GameObject *gameObject)
{
    if (gameObject)
    {
        LOG_TRACE("destruction demandée pour l'objet '%s'.", gameObject->name.c_str());
        gameObject->Destroy();
    }
}

// ProcessDestruction publie maintenant un événement
void Scene::ProcessDestruction()
{
    std::erase_if(m_gameObjects, [&](const std::unique_ptr<GameObject> &go)
                  {
        if(go->IsPendingKill()){
            GameObject* doomed = go.get();
            LOG_INFO("destruction de l'objet '%s'.", doomed->name.c_str());

            // si la caméra active appartient à cet objet, l'invalider avant destruction
            if(m_camera && m_camera->GetGameObject() == doomed){
                m_camera = nullptr;
                LOG_TRACE("caméra active invalidée car son GameObject est détruit.");
            }

            std::erase(m_pendingInitialization, doomed);
            LOG_TRACE("objet '%s' retiré de la file d'initialisation en attente.", doomed->name.c_str());

            m_eventDispatcher->publish(GameObjectWillBeDestroyedEvent(doomed));
            return true;
        }
        return false; });
}

void Scene::Clear()
{
    // 1. Publier les destructions avant de libérer les GameObjects
    for (const auto &go : m_gameObjects)
    {
        if (go)
        {
            m_eventDispatcher->publish(GameObjectWillBeDestroyedEvent(go.get()));
        }
    }

    // 2. Purger les pointeurs non-propriétaires
    m_pendingInitialization.clear();

    // 3. Détruire les objets
    m_gameObjects.clear();

    // 4. Réinitialiser la caméra
    m_camera = nullptr;

    // 5. Notifier les systèmes d'un clear global
    m_eventDispatcher->publish(SceneClearedEvent{});
    LOG_INFO("scène vidée.");
}

void Scene::FlushPendingInitialization()
{
    if (m_pendingInitialization.empty())
    {
        return;
    }

    LOG_TRACE("début du flush d'initialisation (objets en attente=%zu).", m_pendingInitialization.size());

    for (GameObject *go : m_pendingInitialization)
    {
        // pas besoin de vérifier l'existence dans m_gameObjects :
        // ProcessDestruction() retire les objets détruits de m_pendingInitialization
        // via std::erase(), c'est appelé avant FlushPendingInitialization dans la game loop

        if (!go || go->IsPendingKill())
        {
            continue;
        }

        go->MarkInitialized();
        LOG_INFO("objet '%s' initialisé.", go->name.c_str());
        m_eventDispatcher->publish(GameObjectInitializedEvent(go));
    }
    LOG_TRACE("fin du flush d'initialisation.");
    m_pendingInitialization.clear();
}