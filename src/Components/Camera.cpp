#include "Components/Camera.h"
#include "Core/ComponentFactory.h"
#include "Core/GameObject.h"
#include "Core/Scene.h"
#include "Core/Transform.h"
#include "Debug/Logger.h"
#include <nlohmann/json.hpp>

Camera::Camera()
    : m_fov(75.0f),
      m_nearPlane(0.1f),
      m_farPlane(100.0f)
{
}

Vector3 Camera::GetPosition() const
{
    Transform *tr = gameObject ? gameObject->GetComponent<Transform>() : nullptr;
    // warn ?
    return tr ? tr->GetWorldPosition() : Vector3(0.0f, 0.0f, 0.0f);
}

Quaternion Camera::GetOrientation() const
{
    Transform *tr = gameObject ? gameObject->GetComponent<Transform>() : nullptr;
    return tr ? tr->GetOrientation() : Quaternion();
}

Matrix4x4 Camera::GetViewMatrix() const
{
    Transform *tr = gameObject ? gameObject->GetComponent<Transform>() : nullptr;
    if(!tr){
        // warn
        return Matrix4x4::CreateLookAt(
            Vector3(0.0f, 0.0f, 0.0f),
            Vector3(0.0f, 0.0f, -1.0f),
            Vector3(0.0f, 1.0f, 0.0f)
        );
    }

    Vector3 position = tr->GetWorldPosition();
    Quaternion orientation = tr->GetOrientation();

    Vector3 front = orientation * Vector3(0, 0, -1);
    Vector3 up = orientation * Vector3(0, 1, 0);
    front.Normalize();
    up.Normalize();

    return Matrix4x4::CreateLookAt(position, position + front, up);
}

void Camera::UpdateFrustum(const Matrix4x4 &viewProjectionMatrix)
{
    m_frustum.Update(viewProjectionMatrix);
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
        return cam;
    }

    const bool isCameraRegistered = []
    {
        ComponentFactory::Instance().RegisterComponent("Camera", CreateCamera);
        return true;
    }();
}