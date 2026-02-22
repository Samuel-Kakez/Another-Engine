#include "Managers/InputManager.h"
#include "Math/Vector2.h"
#include "Debug/Logger.h"
#include <GLFW/glfw3.h>

InputManager::InputManager(GLFWwindow *window) : m_window(window)
{
    if (!m_window)
    {
        LOG_ERROR("la fenêtre GLFW fournie est nulle.");
    }
    // Initialisation de la position de la souris
    double mouseX, mouseY;
    glfwGetCursorPos(m_window, &mouseX, &mouseY);
    m_currentMousePos = Vector2(static_cast<float>(mouseX), static_cast<float>(mouseY));
    m_lastMousePos = m_currentMousePos;
    m_mouseDelta = Vector2(0.0f, 0.0f);
}

void InputManager::Update()
{
    m_lastMousePos = m_currentMousePos;

    double mouseX, mouseY;
    glfwGetCursorPos(m_window, &mouseX, &mouseY);
    m_currentMousePos = Vector2(static_cast<float>(mouseX), static_cast<float>(mouseY));

    m_mouseDelta = Vector2(m_currentMousePos.x - m_lastMousePos.x,
                           m_lastMousePos.y - m_currentMousePos.y);
}

bool InputManager::IsKeyPressed(int key) const
{
    return glfwGetKey(m_window, key) == GLFW_PRESS;
}

bool InputManager::IsMouseButtonPressed(int button) const
{
    return glfwGetMouseButton(m_window, button) == GLFW_PRESS;
}

Vector2 InputManager::GetMousePosition() const
{
    return m_currentMousePos;
}

Vector2 InputManager::GetMouseDelta() const
{
    return m_mouseDelta;
}

void InputManager::SetCursorMode(int mode)
{
    glfwSetInputMode(m_window, GLFW_CURSOR, mode);
}