#include "Renderer/Camera.h"
#include "Core/ComponentFactory.h"
#include "Core/GameObject.h"
#include "Core/Scene.h"
#include "Core/Transform.h"
#include "Debug/Logger.h"
#include <nlohmann/json.hpp>

Camera::Camera()
    : m_position(0.0f, 0.0f, 0.0f),
      m_orientation(),
      m_fov(75.0f),
      m_nearPlane(0.1f),
      m_farPlane(100.0f),
      m_worldUp(0.0f, 1.0f, 0.0f)
{
    updateCameraVectors();
}

Matrix4x4 Camera::GetViewMatrix() const
{
    // on retourne les membres pré-calculés
    return Matrix4x4::CreateLookAt(m_position, m_position + m_front, m_up);
}

void Camera::updateCameraVectors()
{

    m_front = m_orientation * Vector3(0, 0, -1);
    m_up = m_orientation * Vector3(0, 1, 0);
    m_right = Cross(m_front, m_up);

    m_front.Normalize();
    m_up.Normalize();
    m_right.Normalize();
}

void Camera::UpdateFrustum(const Matrix4x4 &viewProjectionMatrix)
{
    m_frustum.Update(viewProjectionMatrix);
}

void Camera::SetPosition(const Vector3 &position)
{
    m_position = position;
}

void Camera::SetOrientation(const Quaternion &orientation)
{
    m_orientation = orientation;
    updateCameraVectors();
}

namespace
{
    Component *CreateCamera(GameObject *owner, const nlohmann::json &data)
    {
        Camera *cam = owner->AddComponent<Camera>();

        if (data.contains("fov"))
        {
            cam->SetFov(data.value("fov", 75.0f));
        }
        if (data.contains("nearPlane"))
        {
            cam->SetNearPlane(data.value("nearPlane", 0.1f));
        }
        if (data.contains("farPlane"))
        {
            cam->SetFarPlane(data.value("farPlane", 100.0f));
        }

        // Enregistre cette cam comme cam active de la scène
        owner->GetScene().SetActiveCamera(cam);
        LOG_INFO("caméra active définie (FOV=%.0f, near=%.2f, far=%.0f).", cam->GetFov(), cam->GetNearPlane(), cam->GetFarPlane());

        Transform *tr = owner->GetComponent<Transform>();
        if (tr)
        {
            cam->SetPosition(tr->GetPosition());
            cam->SetOrientation(tr->GetOrientation());
        }

        return cam;
    }

    const bool isCameraRegistered = []
    {
        ComponentFactory::Instance().RegisterComponent("Camera", CreateCamera);
        return true;
    }();
}