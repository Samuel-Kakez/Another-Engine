#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <memory> // pour std::unique_ptr
#include <vector>

#include "Renderer/Passes/IRenderPass.h"
#include "Core/EventDispatcher.h"

// Déclarations anticipées
class Scene;
class ResourceManager;
class MeshRenderer;
class Transform;
class Camera;
struct ComponentAddedEvent;
struct GameObjectWillBeDestroyedEvent;
struct Renderable;
class UniformBuffer;
class DirectionalLight;
class SceneBuffer;

/**
 * @brief L'orchestrateur principal du rendu
 * @details Gère la boucle de rendu, y compris les passes multiples comme la passe de profondeur pour les ombres, et la passe de couleur finale.
 *
 */
class Renderer
{

public:
    /**
     * @brief Constructeur, Initialise les shaders et les ressources OpenGL nécessaires au rendu (ex: FBO pour les ombres)
     * @details Il prend maintenant le dispatcher en dépendance, et s'abonne aux événements pour gérer la liste des objets à rendre
     * @param resourceManager référence vers le gestionnaire de ressources
     * @param dispatcher référence vers le dispatcher d'événements
     * @param window fenêtre active pour connaître la taille
     */
    Renderer(ResourceManager &resourceManager, EventDispatcher &dispatcher, GLFWwindow *window);

    /**
     * @brief Destructeur, Libère les ressources OpenGL créées par le Renderer.
     *
     */
    ~Renderer();

    /**
     * @brief La fonction principale de rendu, appelée à chaque image.
     * @param scene La scène à dessiner, contenant tous les objets, lumières et la caméra.
     * @param window la fenêtre GLFW, nécessaire pour obtenir des informations comme la taille de la fenêtre
     */
    void Render(Scene &scene, GLFWwindow *window);

    /**
     * @brief Enregistre un nouvel objet comme étant "dessinable" par le renderer.
     * @param transform
     * @param meshRenderer
     */
    void RegisterRenderable(Transform *transform, MeshRenderer *meshRenderer);

    /**
     * @brief Désenregistre un objet du système de rendu.
     *
     * @param meshRenderer le composant utilisé comme identifiant unique pour trouver l'objet à retirer.
     */
    void UnregisterRenderable(MeshRenderer *meshRenderer);

    /**
     * @brief vide la liste interne des objets à rendre
     * @details méthode essentielle pour reset l'état du renderer lors du chargement d'une nouvelle scène.
     *
     */
    void Clear();

    /**
     * @brief Retourne un pointeur vers la fenêtre GLFW gérée par le renderer
     * @return GLFWwindow*
     */
    GLFWwindow *GetWindow() const { return m_window; }

private:
    // Callbacks pour réagir aux événements
    void OnComponentAdded(const ComponentAddedEvent &event);
    void OnGameObjectDestroyed(const GameObjectWillBeDestroyedEvent &event);
    
    // Stockage pour le désabonnement 
    EventDispatcher* m_dispatcher = nullptr;
    std::vector<EventDispatcher::SubscriptionID> m_subscriptionIDs;

    // --- Données de rendu ---

    /**
     * @brief Cache de toutes les entités "dessinables", optimisé pour un parcours rapide durant les passes de rendu
     * @details Ce vecteur contient des structures "renderable" qui regroupent les pointeurs vers les composants nécessaires pour dessiner un objet.
     */
    std::vector<Renderable> m_renderables;

    /// @brief Structure de données contenant les informations pour le rendu d'une seule frame
    RenderData m_renderData;

    /// @brief Objets visible par la caméra pour la frame en cours, après frustum culling
    std::vector<FrameRenderable> m_frameVisibleRenderables;

    // --- Ressources et Pipeline de Rendu

    /// @brief Séquence de passes de rendu à exécuter (Shadow, Geometry, etc)
    std::vector<std::unique_ptr<IRenderPass>> m_renderPasses;

    // Uniforms Buffer Objects
    std::unique_ptr<UniformBuffer> m_matricesUBO;

    /// @brief Dimensions pour détecter les resize
    int m_lastWidth = 0;
    int m_lastHeight = 0;

    /// @brief Fenêtre glfw
    GLFWwindow* m_window; 


};