#pragma once

#include <memory>
#include <imgui.h>

struct GLFWwindow;

/**
 * @brief Encapsule la logique de l'interface de débogage ImGui.
 *
 */
class DebugUI
{
public:
    /**
     * @brief Constructeur. Initialise ImGui et le lie à GLFW/OpenGL
     *
     * @param window fenêtre GLFW active
     */
    DebugUI(GLFWwindow *window);

    /**
     * @brief Destructeur. Nettoie les ressources ImGUI
     *
     */
    ~DebugUI();

    /**
     * @brief Commence une nouvelle frame ImGui. A appeler avant de définir des fenêtres
     *
     */
    void NewFrame();

    /**
     * @brief Dessine la fenêtre de statistiques de rendu
     *
     * @param deltaTime Le temps de la dernière frame en secondes.
     */
    void DrawStatsWindow(float deltaTime);

    /**
     * @brief Dessine la fenêtre de logs du moteur
     */
    void DrawLogWindow();

    /**
     * @brief Effectue le rendu de toutes les commandes ImGui définies.
     *
     */
    void Render();

private:
    // variables pour le lissage
    float m_smoothedDeltaTime = 0.0f;
    bool m_isFirstFrame = true;

    // filtres pour la fenêtre de logs
    ImGuiTextFilter m_logFilter;
    bool m_autoScroll = true;
    GLFWwindow *m_window = nullptr;
    float m_logWindowHeight = 250.0f;

    // Filtres par niveau
    bool m_showTrace = true;
    bool m_showInfo = true;
    bool m_showWarn = true;
    bool m_showError = true;
};