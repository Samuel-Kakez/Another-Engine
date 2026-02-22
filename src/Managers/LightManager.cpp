#include <glad/glad.h>

#include "Managers/LightManager.h"
#include "Debug/Logger.h"

// inclusions pour le système d'événements
#include "Core/EventDispatcher.h"
#include "Core/GameObject.h"
#include "Components/DirectionalLight.h"
#include "Renderer/RenderSettings.h"
#include "Core/Scene.h"

// Implémentation du constructeur qui s'abonne aux événements
LightManager::LightManager(EventDispatcher &dispatcher)
{
    m_dispatcher = &dispatcher;

    m_subscriptionIDs.push_back(
        dispatcher.subscribe<ComponentAddedEvent>([this](const ComponentAddedEvent &event)
                                                  { this->OnComponentAdded(event); }));

    m_subscriptionIDs.push_back(
        dispatcher.subscribe<GameObjectWillBeDestroyedEvent>([this](const GameObjectWillBeDestroyedEvent &event)
                                                             { this->OnGameObjectDestroyed(event); }));

    m_subscriptionIDs.push_back(
        dispatcher.subscribe<SceneClearedEvent>([this](const SceneClearedEvent &)
                                                { this->Clear(); }));
}

LightManager::~LightManager()
{
    if (m_dispatcher)
    {
        for (auto id : m_subscriptionIDs)
        {
            m_dispatcher->unsubscribe(id);
        }
    }
    Clear(); // S'assure que tout est détruit à la fin
}

// Implémentation des callbacks
void LightManager::OnComponentAdded(const ComponentAddedEvent &event)
{
    if (DirectionalLight *light = dynamic_cast<DirectionalLight *>(event.component))
    {
        RegisterLight(light);
    }
}

void LightManager::OnGameObjectDestroyed(const GameObjectWillBeDestroyedEvent &event)
{
    if (DirectionalLight *light = event.gameObject->GetComponent<DirectionalLight>())
    {
        UnregisterLight(light);
    }
}

void LightManager::RegisterLight(DirectionalLight *light)
{
    m_DirectionalLight = light;
    // Crée la shadowmap pour la lumière
    if (light->castsShadows)
    {
        CreateShadowResourcesFor(light);
    }
    LOG_INFO("lumière directionnelle enregistrée (ombres: %s, résolution: %u).", light->castsShadows ? "oui" : "non", light->shadowResolution);
}

void LightManager::UnregisterLight(DirectionalLight *light)
{
    // Détruit les ressources de la light
    DestroyShadowResourcesFor(light);
    if (m_DirectionalLight == light)
    {
        m_DirectionalLight = nullptr;
    }
    LOG_INFO("lumière directionnelle désenregistrée.");
}

DirectionalLight *LightManager::GetDirectionalLight() const
{
    return m_DirectionalLight;
}

void LightManager::Clear()
{
    // Parcorut toutes les lumières pour détruire leurs ressources avant de vider le vecteur
    if (m_DirectionalLight)
    {
        DestroyShadowResourcesFor(m_DirectionalLight);
        m_DirectionalLight = nullptr;
    }
}

// --- Gestion des ressources d'ombres ---

void LightManager::CreateShadowResourcesFor(DirectionalLight *light)
{
    // On ne fait rien si la lumière est nulle ou a déjà  une cubemap
    if (light == nullptr || light->shadowMap != 0)
    {
        return;
    }

    // Génération de la texture
    glGenTextures(1, &light->shadowMap);
    glBindTexture(GL_TEXTURE_2D, light->shadowMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, light->shadowResolution, light->shadowResolution, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

    // On définit les paramètres de la texture
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    float borderColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    glBindTexture(GL_TEXTURE_2D, 0); // On délie la texture
    LOG_INFO("shadow map créée (%ux%u).", light->shadowResolution, light->shadowResolution);
}

void LightManager::DestroyShadowResourcesFor(DirectionalLight *light)
{
    // On ne supprime la texture que si elle existe
    if (light != nullptr && light->shadowMap != 0)
    {
        glDeleteTextures(1, &light->shadowMap);
        light->shadowMap = 0;
    }
}