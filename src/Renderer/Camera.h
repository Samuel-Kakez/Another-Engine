#pragma once

#include "Math/Matrix4x4.h"
#include "Math/Vector3.h"
#include "Math/Quaternion.h"
#include "Math/Frustum.h"
#include "Core/Component.h"

/**
 * @brief Représente un point de vue fixe dans la scène 3D
 * @details Cette classe calcule la matrice de vue (View Matrix) nécessaire au rendu.
 *
 */
class Camera : public Component
{
public:
    /**
     * @brief Constructeur par défaut
     *
     */
    Camera();

    // --- Getters ---
    const Vector3 &GetPosition() const { return m_position; }
    const Quaternion &GetOrientation() const { return m_orientation; }
    float GetFov() const { return m_fov; }
    float GetNearPlane() const { return m_nearPlane; }
    float GetFarPlane() const { return m_farPlane; }
    const Frustum &GetFrustum() const { return m_frustum; }

    // --- Setters ---
    void SetPosition(const Vector3 &position);
    void SetOrientation(const Quaternion &orientation);
    void SetFov(float fov) { m_fov = fov; }
    void SetNearPlane(float nearPlane) { m_nearPlane = nearPlane; }
    void SetFarPlane(float farPlane) { m_farPlane = farPlane; }

    /**
     * @brief Calcule et retourne la matrice de vue (view matrix)
     *
     * @return Matrix4x4
     */
    Matrix4x4 GetViewMatrix() const;

    /**
     * @brief Met à jour le frustum de la caméra
     *
     * @param viewProjectionMatrix La matrice combinée de vue et de projection
     */
    void UpdateFrustum(const Matrix4x4 &viewProjectionMatrix);

private:
    // --- Propriétés de la caméra ---
    Vector3 m_position;
    Quaternion m_orientation;
    float m_fov;
    float m_nearPlane;
    float m_farPlane;

    // --- Vecteurs directionnels ---
    Vector3 m_front;
    Vector3 m_up;
    Vector3 m_right;
    Vector3 m_worldUp;
    Frustum m_frustum;

    /**
     * @brief Recalcule les vecteurs directionnels (front, right, up) à partir du quaternion d'orientation
     * @details Doit être appelé après chaque modification du quaternion "orientation"
     */
    void updateCameraVectors();
};