#include "Components/FreeCamController.h"
#include "Core/GameObject.h"
#include "Core/Scene.h"
#include "Managers/InputManager.h"
#include "Components/Camera.h"
#include "Core/ComponentFactory.h"
#include "Core/Transform.h"

#include "Math/Quaternion.h"
#include "Math/Vector3.h"

#include <nlohmann/json.hpp>
#include <GLFW/glfw3.h>
#include <algorithm>

FreeCamController::FreeCamController(float moveSpeed, float lookSensitivity) : m_moveSpeed(moveSpeed), m_lookSensitivity(lookSensitivity)
{
}

void FreeCamController::Update(float deltaTime)
{

    Camera *camera = gameObject->GetScene().GetCamera();
    if (!camera)
        return;

    Transform *camTransform = camera->gameObject->GetComponent<Transform>();
    if (!camTransform)
        return;

    // Initialisation unique au premier appel
    if (!m_IsInitialized)
    {
        Vector3 initialEuler = camTransform->GetOrientation().ToEulerAngles();
        m_pitch = initialEuler.x;
        m_yaw = initialEuler.y;
        m_IsInitialized = true;
    }

    InputManager &input = gameObject->GetScene().GetInputManager();

    // Bascule le mode de contrôle avec clic droit
    bool isRightMouseButtonPressed = input.IsMouseButtonPressed(GLFW_MOUSE_BUTTON_RIGHT);
    if (isRightMouseButtonPressed && !m_wasRightMouseButtonPressed)
    {
        m_isControlEnabled = !m_isControlEnabled;
        input.SetCursorMode(m_isControlEnabled ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
    }
    m_wasRightMouseButtonPressed = isRightMouseButtonPressed;

    // Si le contrôle est inactif, on sort ici
    if (!m_isControlEnabled)
    {
        return;
    }

    // 1. Rotation
    Vector2 mouseDelta = input.GetMouseDelta();
    m_yaw -= mouseDelta.x * m_lookSensitivity;
    m_pitch += mouseDelta.y * m_lookSensitivity;

    // Contrainte du pitch pour éviter le retournement
    m_pitch = std::clamp(m_pitch, -89.9f, 89.9f);

    Quaternion orientation = Quaternion::FromEulerAngles(Vector3(m_pitch, m_yaw, 0.0f));
    camTransform->SetOrientation(orientation);

    // 2. Déplacement
    Vector3 position = camTransform->GetPosition();
    const float currentSpeed = m_moveSpeed * deltaTime;
    const Vector3 front = orientation * Vector3(0, 0, -1);
    Vector3 right = Cross(front, Vector3(0, 1, 0));
    right.Normalize();

    if (input.IsKeyPressed(GLFW_KEY_W))
        position = position + (front * currentSpeed);
    if (input.IsKeyPressed(GLFW_KEY_S))
        position = position - (front * currentSpeed);

    if (input.IsKeyPressed(GLFW_KEY_A))
        position = position - (right * currentSpeed);
    if (input.IsKeyPressed(GLFW_KEY_D))
        position = position + (right * currentSpeed);

    if (input.IsKeyPressed(GLFW_KEY_SPACE))
        position = position + (Vector3(0, 1, 0) * currentSpeed);
    if (input.IsKeyPressed(GLFW_KEY_LEFT_CONTROL))
        position = position - (Vector3(0, 1, 0) * currentSpeed);

    camTransform->SetPosition(position);
}

// Enregistrement auprès de la factory
namespace
{
    Component *CreateFreeCamController(GameObject *owner, const nlohmann::json &data)
    {
        float speed = data.value("speed", 5.0f);
        float sensitivity = data.value("sensitivity", 0.1f);
        return owner->AddComponent<FreeCamController>(speed, sensitivity);
    }
    const bool isFreeCamControllerRegistered = []
    {
        ComponentFactory::Instance().RegisterComponent("FreeCamController", CreateFreeCamController);
        return true;
    }();

}