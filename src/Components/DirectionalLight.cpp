#include "Components/DirectionalLight.h"
#include "Core/ComponentFactory.h"
#include "Core/GameObject.h"

#include <nlohmann/json.hpp>

/// @brief Constructeur par défaut
DirectionalLight::DirectionalLight() : castsShadows(true), shadowResolution(2048)
{
}

/// @brief Constructeur paramétré
/// @param color 
/// @param intensity 
/// @param castsShadows 
/// @param shadowResolution 
DirectionalLight::DirectionalLight(bool castsShadows, unsigned int shadowResolution) : castsShadows(castsShadows), shadowResolution(shadowResolution)
{
}

namespace
{
    /// @brief Factory function pour créer une DirectionalLight depuis le JSON
    /// @param owner GameObject propriétaire du composant
    /// @param data Données JSON contenant les paramètres de la lumière
    /// @return Pointeur vers le composant créé
    Component *CreateDirectionalLight(GameObject *owner, const nlohmann::json &data)
    {
        bool castsShadows = data.value("castsShadows", true);
        unsigned int shadowResolution = data.value("shadowResolution", 2048);

        return owner->AddComponent<DirectionalLight>(castsShadows, shadowResolution);
    }

    /// @brief Enregistrement automatique du composant dans la factory
    // Permet la création du DirectionalLight
    const bool isDirectionalLightRegistered = []
    {
        ComponentFactory::Instance().RegisterComponent("DirectionalLight", CreateDirectionalLight);
        return true;
    }();
}