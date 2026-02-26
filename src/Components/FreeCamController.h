#pragma once

#include "Core/Component.h"

/**
 * @brief Composant de débug pour contrôler la caméra en mode "vol"*
 * @details Activé/Désactivé par clic droit
 *
 */
class FreeCamController : public Component
{
public:
    /**
     * @brief Constructeur par défaut
     *
     * @param moveSpeed vitesse de déplacement
     * @param lookSensivity sensibilité de la caméra
     */
    FreeCamController(float moveSpeed = 5.0f, float lookSensivity = 0.1f);

    /**
     * @brief Met à jour la logique du contrôleur à chaque frame
     *
     * @param deltaTime temps écoulé depuis la dernière frame
     */
    void Update(float deltaTime) override;

private:
    // Paramètres de contrôle
    float m_moveSpeed;
    float m_lookSensitivity;

    // État interne
    bool m_IsInitialized = false;
    bool m_isControlEnabled = false;
    float m_yaw = 0.0f;
    float m_pitch = 0.0f;
    bool m_wasRightMouseButtonPressed = false;
};