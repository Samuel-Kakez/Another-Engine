#pragma once

#include "Debug/DebugUI.h"

#include <memory>

// on ne met que des déclarations anticipées pour garder le header léger et éviter les dépendances circulaires
class GLFWwindow;
class Renderer;
class Scene;
class ResourceManager;
class LightManager;
class InputManager;

/**
 * @brief La classe principale qui orchestre l'ensemble du moteur
 * @details Cette classe est responsable de l'initialisation des systèmes (GLFW, GLAD),
 * de la création et de la possession des principaux managers (ResourceManager, LightManager),
 * du Renderer et de la Scene. Elle contient également la boucle de jeu principale
 *
 */
class Engine
{
public:
    /**
     * @brief Constructeur de l'Engine
     * @details Initialise les bibliothèques, créé la fenêtre et instancie tous les systèmes du moteur
     *
     */
    Engine();

    /**
     * @brief Destructeur de l'Engine
     * @details Assure le nettoyage des ressources et la terminaison propre des bibliothèques
     */
    ~Engine();

    /**
     * @brief Exécute la boucle de jeu principale
     *
     */
    void Run();

    /**
     * @brief Rend une seule frame
     * @details Utilisé principalement lors du redimensionnement de la fenêtre
     */
    void RenderOneFrame();

private:
    // Fenêtre GLFW
    GLFWwindow *m_window;
    // Etat d'initialisation du moteur
    bool m_initialized = false;

    // systèmes principaux du moteur, possédés par des unique_ptr pour une gestion automatique de la mémoire.
    std::unique_ptr<ResourceManager> m_resourceManager;
    std::unique_ptr<Scene> m_scene;
    std::unique_ptr<LightManager> m_lightManager;
    std::unique_ptr<InputManager> m_inputManager;
    std::unique_ptr<Renderer> m_renderer;
    std::unique_ptr<DebugUI> m_debugUI;
};