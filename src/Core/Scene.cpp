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
        gameObject->Destroy();
    }
}

// ProcessDestruction publie maintenant un événement
void Scene::ProcessDestruction()
{
    std::erase_if(m_gameObjects, [&](const std::unique_ptr<GameObject> &go)
                  {
        if(go->IsPendingKill()){
            // On publie l'événement avant de retourner true (ce qui causera la destruction)
            // On dit explicitement "créé un gameObjectWillBeDestroyedEvent et initialise son membre 'gameObject' avec la valeur go.get()
            m_eventDispatcher->publish(GameObjectWillBeDestroyedEvent(go.get()));
            return true;
        }
        return false; });
}

void Scene::Clear()
{
    // 1. On vide la liste des gameObjects. La destruction des unique_ptr s'occupe de libérer la mémoire pour chaque GameObject et ses composants.
    m_gameObjects.clear();

    // 2. On réinitialise la caméra à un état par défaut.
    m_camera = nullptr;

    // 3. On publie un événement pour notifier tous les systèmes abonnés.
    m_eventDispatcher->publish(SceneClearedEvent{});
    LOG_INFO("scène vidée.");
}
