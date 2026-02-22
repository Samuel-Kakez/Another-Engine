#pragma once

#include <vector>
#include <memory>
#include <string>
#include "Math/Vector3.h"
#include "Renderer/RenderSettings.h"

// Déclarations anticipées pour éviter d'inclure les en-têtes complets ici
// Bonne pratique pour accélérer la compilation
class GameObject;
class Camera;
class ResourceManager;
class LightManager;
class InputManager;
class Component;
class Renderer;
class EventDispatcher;

/**
 * @brief Représente un monde virtuel contenant des GameObjects et une caméra
 * @details La Scène est le conteneur principal qui possède tous les objets du jeu
 * Elle gère leur cycle de vie (création, mise à jour, destruction)
 *
 */
class Scene
{
public:
    /**
     * @brief Constructeur. Ne prend plus tous les managers, mais seulement ceux dont il a besoin,
     * ainsi que le dispatcher d'événements dont il prend possession.
     *
     * @param dispatcher pointeur unique vers le dispatcher d'événements
     * @param resourceManager référence vers le gestionnaire de ressources
     * @param lightManager  référence vers le gestionnaire des lights
     * @param inputManager référence vers le gestionnaire d'inputs
     * @param renderer référence vers le renderer
     */
    Scene(std::unique_ptr<EventDispatcher> dispatcher, ResourceManager &resourceManager, LightManager &lightManager, InputManager &inputManager, Renderer &renderer);

    /**
     * @brief La destruction est désormais automatique, le destructeur est inutile.
     *
     */
    ~Scene();

    /**
     * @brief Met à jour tous les GameObjects de la scène
     * @param deltaTime le temps écoulé depuis la dernière frame en secondes
     */
    void Update(float deltaTime);

    /**
     * @brief Créé un nouvel GameObject, l'ajoute à la scène et transfère la propriété
     * @details Nouvelle méthode pour créer des objets dans la scène. Elle garantit que l'objet sera correctement géré et détruit
     * @param name le nom à assigner au nouveau GameObject
     * @return GameObject* Un pointeur brut (non-propriétaire) vers l'objet créé, pour sa configuration initiale
     */
    GameObject *AddGameObject(const std::string &name = "GameObject");

    /**
     * @brief Trouve un GameObject dans la scène par son nom.
     * @details Utile pour lier des objets entre eux, par exemple lors de la désérialisation.
     * La recherche est linéaire, à ne pas utiliser dans des boucles de performance
     *
     * @param name le nom du GameObject à trouver
     * @return GameObject* un pointeur vers l'objet trouvé, ou nullptr s'il n'existe pas
     */
    GameObject *FindGameObjectByName(const std::string &name);

    /**
     * @brief Demande la destruction d'un GameObject à la fin de la frame
     * @details La destruction n'est pas immédiate pour éviter les problèmes d'invalidation d'itérateurs
     * @param gameObject le pointeur vers l'objet à détruire
     */
    void DestroyGameObject(GameObject *gameObject);

    /**
     * @brief Fournit un accès en lecture seule à la caméra de la scène
     *
     * @return Camera* un pointeur brut (non-propriétaire) vers l'objet Camera
     */
    Camera *GetCamera() const { return m_camera; }

    /// @brief Définit la cam active de la scène
    /// @param camera 
    void SetActiveCamera(Camera* camera) {m_camera = camera;}

    /**
     * @brief Fournit un accès en lecture seule à la liste des gameObjects
     *
     * @return une référence constante vers le vecteur de pointeurs uniques de GameObjects
     */
    const std::vector<std::unique_ptr<GameObject>> &GetGameObjects() const { return m_gameObjects; }

    /**
     * @brief Fournit l'accès au ResourceManager de la scène
     *
     * @return ResourceManager& Une référence vers le manager resources
     */
    ResourceManager &GetResourceManager() { return m_resourceManager; }

    /**
     * @brief Fournit l'accès au LightManager de la scène
     *
     * @return LightManager& une référence vers le manager light
     */
    LightManager &GetLightManager() { return m_lightManager; }

    /**
     * @brief Fournit l'accès au InputManager de la scène
     *
     * @return InputManager& une référence vers le manager input
     */
    InputManager &GetInputManager() const { return m_inputManager; }

    /**
     * @brief Fournit l'accès au Renderer de la scène
     *
     * @return Renderer& Une référence vers le renderer
     */
    Renderer &GetRenderer() { return m_renderer; }

    /**
     * @brief Getter pour le dispatcher
     *
     * @return EventDispatcher&
     */
    EventDispatcher &GetEventDispatcher() { return *m_eventDispatcher; }

    /**
     * @brief Vide complètement la scène de ses GameObjects et publie un événement de nettoyage.
     * @details Cette méthode nettoie uniquement ce que la scène possède directement (GameObjects, Caméra)
     * et publie un événement SceneClearedEvent pour que les autres systèmes puissent réinitialiser leur propre état.
     */
    void Clear();

    /**
     * @brief Met à jour tous les GameObjects de la scène à un pas de temps fixe
     *
     * @param fixedDeltaTime le pas de temps fixe
     */
    void FixedUpdate(float fixedDeltaTime);

    const RenderSettings &GetRenderSettings() const { return renderSettings; }
    RenderSettings &GetRenderSettings() { return renderSettings; }

private:
    friend class Engine; // Donne à Engine l'accès aux membres privés de Scene

    RenderSettings renderSettings;

    // La scène est propriétaire des GameObjects
    std::vector<std::unique_ptr<GameObject>> m_gameObjects;
    Camera* m_camera = nullptr;

    // La scène possède le dispatcher
    std::unique_ptr<EventDispatcher> m_eventDispatcher;

    // Références non-propriétaires vers les managers fournis par l'Engine
    Renderer &m_renderer;
    ResourceManager &m_resourceManager;
    LightManager &m_lightManager;
    InputManager &m_inputManager;

    /**
     * @brief Nettoie les objets marqués pour destruction. Appelé par l'Engine en fin de frame.
     *
     */
    void ProcessDestruction();
};