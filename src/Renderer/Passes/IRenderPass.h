#pragma once

#include <vector>
#include <memory>
#include "Math/Matrix4x4.h"
#include "Math/AABB.h"
#include "Renderer/RenderSettings.h"
#include "Components/DirectionalLight.h"

class Scene;
class ResourceManager;
class Camera;
class Transform;
class MeshRenderer;
class UniformBuffer;
class DirectionalLight;

/// @brief Structure interne pour stocker tout ce qui est nécessaire au rendu d'un objet
struct Renderable
{
    Transform *transform;
    MeshRenderer *meshRenderer;
};

/// @brief Structure contenant un Renderable et les données AABB (bounding-box)
struct FrameRenderable
{
    Renderable renderable;
    AABB worldAABB;
};

/// @brief Conteneur de données partagées entre les passes de rendu pour une frame.
struct RenderData
{
    Scene *scene = nullptr;
    Camera *camera = nullptr;
    ResourceManager *resourceManager = nullptr;

    int screenWidth = 0;
    int screenHeight = 0;

    UniformBuffer *matricesUBO = nullptr;

    std::vector<FrameRenderable> *cameraVisibleRenderables = nullptr;
    const std::vector<Renderable> *allRenderables = nullptr;

    DirectionalLight *directionalLight = nullptr;

    /// @brief Matrices light-space pour chaque cascade (VP lumière)
    Matrix4x4 lightSpaceMatrices[DirectionalLight::NUM_CASCADES];

    /// @brief Distances des plans de séparation des cascades (clip-space Z)
    float cascadePlaneDistances[DirectionalLight::NUM_CASCADES] = {};

    const RenderSettings *renderSettings = nullptr;
};

/// @brief Interface pour une passe de rendu
class IRenderPass
{
public:
    virtual ~IRenderPass() = default;
    virtual void Execute(RenderData &data) = 0;
    virtual bool isEnabled(const RenderSettings &settings) const = 0;
};