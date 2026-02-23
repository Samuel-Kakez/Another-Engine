#pragma once

#include "Math/Vector3.h"

/// @brief Paramètres de la projection de la shadow map
struct ShadowSettings
{
    /// @brief Distance maximale à laquelle les ombres sont rendues
    float maxShadowDistance = 100.0f;

    /// @brief Multiplicateur lambda pour le split scheme (0 = uniform, 1 = logarithmic)
    // 0.5 est un bon compromis
    float cascadeSplitLambda = 0.5f;
};

/// @brief Paramètres d'éclairage ambient hémisphérique
struct AmbientSettings
{
    Vector3 skyColorZenith{0.2f, 0.4f, 0.8f};
    Vector3 skyColorHorizon{0.6f, 0.7f, 0.9f};
    Vector3 groundColor{0.1f, 0.08f, 0.06f};
    float intensity = 0.15f;
};

/// @brief Configuration globale du rendu
struct RenderSettings
{
    bool enableShadows = true;

    Vector3 lightColor{1.0f, 0.95f, 0.9f};
    float lightIntensity = 5.0f;

    /// @brief Exposition manuelle (multiplie le HDR avant tone mapping)
    float exposure = 1.0f;

    ShadowSettings shadow;
    AmbientSettings ambient;
};