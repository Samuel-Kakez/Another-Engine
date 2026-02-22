#pragma once

#include "Renderer/Passes/IRenderPass.h"
#include <memory>

class FrameBuffer;

/// @class ShadowPass
/// @brief Passe de rendu pour générer la shadow map directionnelle

// Cette passe effectue un rendu de la scène depuis le POV de la lumière
// dans une texture de profondeur.

// Pour éviter les artefacts, on utilise glPolygonOffset (biais matériel) et le culling des faces avant
class ShadowPass : public IRenderPass
{
public:
    /// @brief Constructeur
    ShadowPass();
    /// @brief Destructeur
    ~ShadowPass();
    /// @brief Exécute la passe de shadow mapping
    /// @param data Données de rendu de la scène & paramètres
    void Execute(RenderData &data) override;

    /// @brief Vérifie si les ombres sont activées dans les settings
    /// @param settings 
    /// @return 
    bool isEnabled(const RenderSettings &settings) const override {
        return settings.enableShadows;
    }

private:
    /// @brief Framebuffer dédié au rendu de la shadow map
    std::unique_ptr<FrameBuffer> m_shadowFBO;
};