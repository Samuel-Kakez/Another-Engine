#include <algorithm>

#include "Core/Scene.h"
#include "Core/GameObject.h"
#include "Core/Transform.h"
#include "Core/Component.h"
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

            // Retire aussi de la file d'initialisation pour éviter un pointeur perdant
            std::erase(m_pendingInitialization, doomed);
            LOG_TRACE("objet '%s' retiré de la file d'initialisation en attente.", doomed->name.c_str());

            m_eventDispatcher->publish(GameObjectWillBeDestroyedEvent(doomed));
            return true;
        }
        return false; });
}

void Scene::Clear()
{

    // Purge des pointeurs non-propriétaires avant destruction des objets
    m_pendingInitialization.clear();
    // 1. On vide la liste des gameObjects. La destruction des unique_ptr s'occupe de libérer la mémoire pour chaque GameObject et ses composants.
    m_gameObjects.clear();

    // 2. On réinitialise la caméra à un état par défaut.
    m_camera = nullptr;

    // 3. On publie un événement pour notifier tous les systèmes abonnés.
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
        if (!go)
        {
            continue;
        }

        auto it = std::find_if(
            m_gameObjects.begin(),
            m_gameObjects.end(),
            [go](const std::unique_ptr<GameObject> &obj)
            { return obj.get() == go; });

        if (it == m_gameObjects.end())
        {
            LOG_TRACE("initialisation ignorée : pointeur d'objet absent de la scène.");
            continue;
        }

        if (go->IsPendingKill())
        {
            LOG_TRACE("initialisation ignorée pour '%s' : objet en attente de destruction.", go->name.c_str());
            continue;
        }

        go->MarkInitialized();
        LOG_INFO("objet '%s' initialisé.", go->name.c_str());
        m_eventDispatcher->publish(GameObjectInitializedEvent(go));
    }
    LOG_TRACE("fin du flush d'initialisation.");
    m_pendingInitialization.clear();
}