#pragma once

#include "Math/Vector2.h"

struct GLFWwindow;

/**
 * @brief Gère de manière centralisée les entrées utilisateur (clavier, souris)
 * @details Cette classe est instanciée et possédée par l'Engine.
 */
class InputManager
{
public:
    /**
     * @brief Constructeur par défaut
     *
     * @param window pointeur vers la fenêtre GLFW principale
     */
    explicit InputManager(GLFWwindow *window);

    /**
     * @brief Met à jour l'état des données, appelé à chaque frame
     *
     */
    void Update();

    /**
     * @brief Vérifie si une touche du clavier est actuellement enfoncée
     *
     * @param key le code de la touche

     */
    bool IsKeyPressed(int key) const;

    /**
     * @brief Vérifie si un bouton de la souris est actuellement enfoncé
     *
     * @param button le code du bouton
     * @return true ou false
     */
    bool IsMouseButtonPressed(int button) const;

    /**
     * @brief Récupère la position actuelle du curseur de la souris
     *
     * @return Vector2
     */
    Vector2 GetMousePosition() const;

    /**
     * @brief Récupère le déplacement de la souris depuis la dernière frame
     *
     * @return Vector2
     */
    Vector2 GetMouseDelta() const;

    /**
     * @brief Définit le mode du curseur (normal, caché, capturé)
     *
     * @param mode
     */
    void SetCursorMode(int mode);

private:
    GLFWwindow *m_window = nullptr;
    Vector2 m_currentMousePos;
    Vector2 m_lastMousePos;
    Vector2 m_mouseDelta;
};