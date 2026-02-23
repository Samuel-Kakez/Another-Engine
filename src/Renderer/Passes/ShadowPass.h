#pragma once

#include "Renderer/Passes/IRenderPass.h"
#include <vector>
#include <memory>

#include "Math/Vector3.h"
#include "Math/Matrix4x4.h"

class FrameBuffer;

/// @class ShadowPass
/// @brief Passe de rendu pour générer les shadow maps directionnelles en Cascaded Shadow Mapping (CSM)
/// Cette passe découpe le frustum de la caméré en NUM_CASCADES sous-frustums, calcule une projection orthographique
/// ajustée pour chacun, puis rend toute la scène dans un GL_TEXTURE_2D_ARRAY via un geometry shader qui route chaque triangle vers le layer approprié.
class ShadowPass : public IRenderPass
{
public:
    ShadowPass();
    ~ShadowPass();

    void Execute(RenderData &data) override;

    bool isEnabled(const RenderSettings &settings) const override
    {
        return settings.enableShadows;
    }

private:
    std::unique_ptr<FrameBuffer> m_shadowFBO;

    /// @brief Calcule les 8 coins world-space d'un sous-frustum défini par nearPlane, farPlane
    /// @param view
    /// @param fovY
    /// @param aspect
    /// @param nearPlane
    /// @param farPlane
    /// @return
    std::vector<Vector3> GetFrustumCornersWorldSpace(
        const Matrix4x4 &view,
        float fovY, float aspect,
        float nearPlane, float farPlane) const;

    /// @brief Construit la matrice lightView * lightProjection pour une cascade donnée
    /// @details Calcule l'AABB des coins du frustum dans l'espace lumière, construit une projection orthographique ajustée au plus serré, puis
    /// Applique un textel snapping pour éviter le shadow swimming lors de mouvements de la caméra.
    /// @param frustumCorners les 8 coins world-space du sous-frustum de la cascade
    /// @param lightDir la direction normalisée de la lumière (forward du transform)
    /// @param shadowResolution résolution de la shadow map (pour le textel snapping)
    /// @return la matrice combinée lightProjection * lightView pour cette cascade
    Matrix4x4 ComputeCascadeMatrix(
        const std::vector<Vector3> &frustumCorners,
        const Vector3 &lightDir,
        unsigned int shadowResolution) const;
};