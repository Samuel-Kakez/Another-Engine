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
    if (light == nullptr || light->shadowMapArray != 0)
    {
        return;
    }

    // --- Création de la texture array pour le Cascaded Shadow Mapping ---
    // GL_TEXTURE_2D_ARRAY : une texture 3D où chaque "tranche" est une shadow map de cascade
    // Chaque layer a la même résolution
    // Format GL_DEPTH_COMPONENT32F pour la précision en profondeur
    glGenTextures(1, &light->shadowMapArray);
    glBindTexture(GL_TEXTURE_2D_ARRAY, light->shadowMapArray);

    // 4 layers, un par cascade
    glTexImage3D(
        GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT32F,
        light->shadowResolution, light->shadowResolution,
        DirectionalLight::NUM_CASCADES,
        0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

        // NEAREST filtering : pas d'interpolation sur les valeurs de profondeur (le PCF se fait dans le shader)
        // CLAMP_TO_BORDER avec couleur blanche (1.0) : hors-frustum -> profondeur max -> pas d'ombre
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    float borderColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
    glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, borderColor);

    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
    LOG_INFO("CSM shadow map array créée (%ux%u, %d cascades).", light->shadowResolution, light->shadowResolution, DirectionalLight::NUM_CASCADES);
}

void LightManager::DestroyShadowResourcesFor(DirectionalLight *light)
{
    // On ne supprime la texture que si elle existe
    if (light != nullptr && light->shadowMapArray != 0)
    {
        glDeleteTextures(1, &light->shadowMapArray);
        light->shadowMapArray = 0;
    }
}