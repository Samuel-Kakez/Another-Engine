#include "Renderer/Passes/ShadowPass.h"
#include "Renderer/Buffers/FrameBuffer.h"
#include "Components/DirectionalLight.h"
#include "Core/Transform.h"
#include "Core/Scene.h"
#include "Core/GameObject.h"
#include "Managers/ResourceManager.h"
#include "Managers/LightManager.h"
#include "Renderer/Shader.h"
#include "Components/MeshRenderer.h"
#include "Math/Matrix4x4.h"
#include "Components/Camera.h"
#include "Renderer/RenderSettings.h"

#include <glad/glad.h>
#include <cmath>
#include <algorithm>
#include <limits>

ShadowPass::ShadowPass()
{
    m_shadowFBO = std::make_unique<FrameBuffer>();
}

ShadowPass::~ShadowPass() = default;

// ============================================================
// Helpers - Calcul des frustums et matrices de cascade
// ============================================================

/// @brief Construit une matrice perspective pour le sous-frustum [nearPlane, farPlane], puis inverse la VP pour projeter les
/// 8 coins NDC (-1...+1) en world-space. Les coins sont renvoyés dans un ordre itératif (x, y, z ∈ {0,1}).
std::vector<Vector3> ShadowPass::GetFrustumCornersWorldSpace(const Matrix4x4 &view,
                                                             float fovY, float aspect,
                                                             float nearPlane, float farPlane) const
{
    // Génère une perspective de projection pour [nearPlane, farPlane]
    Matrix4x4 proj = Matrix4x4::CreatePerspectiveProjection(fovY, aspect, nearPlane, farPlane);

    // Inverse de la matrice View-Projection : permet de reprojeter les coins NDC vers le world-space
    Matrix4x4 inv = (proj * view).Inverse();

    // 8 corners
    std::vector<Vector3> corners;
    corners.reserve(8);
    for (int x = 0; x <= 1; ++x)
    {
        for (int y = 0; y <= 1; ++y)
        {
            for (int z = 0; z <= 1; ++z)
            {
                Vector3 ndc(2.0f * x - 1.0f, 2.0f * y - 1.0f, 2.0f * z - 1.0f);
                corners.push_back(inv.TransformPoint(ndc));
            }
        }
    }
    return corners;
}

Matrix4x4 ShadowPass::ComputeCascadeMatrix(
    const std::vector<Vector3> &frustumCorners,
    const Vector3 &lightDir,
    unsigned int shadowResolution) const
{
    // --- Étape 1 : Centroîde du sous-frustum ---
    // Moyenne des 8 coins pour positionner la caméra-lumière au centre du volume
    Vector3 center(0.0f, 0.0f, 0.0f);
    for (const auto &c : frustumCorners)
    {
        center += c;
    }
    center /= static_cast<float>(frustumCorners.size());

    // --- Étape 2 : Matrice de vue lumière ---
    // Choix du vecteur up : si la lumière est quasi-verticale, on utilise Z comme up
    // pour éviter la dégénerescence du cross product (à creuser personnellement, ca m'intéresse!)
    Vector3 lightUp;
    if (std::abs(lightDir.y) > 0.99f)
    {
        lightUp = Vector3(0.0f, 0.0f, 1.0f);
    }
    else
    {
        lightUp = Vector3(0.0f, 1.0f, 0.0f);
    }

    // La caméra-lumière se place à (center - lightDir) et regarde vers center
    // La distance exacte n'importe pas en projection ortho, seule la direction compte.
    Matrix4x4 lightView = Matrix4x4::CreateLookAt(center - lightDir, center, lightUp);

    // --- Étape 3 : AABB des coins du frustum dans l'espace lumière ---
    // On transforme chaque coin en light-view space et on accumule les min/max
    float minX = std::numeric_limits<float>::max();
    float maxX = -std::numeric_limits<float>::max();
    float minY = std::numeric_limits<float>::max();
    float maxY = -std::numeric_limits<float>::max();
    float minZ = std::numeric_limits<float>::max();
    float maxZ = -std::numeric_limits<float>::max();

    // Transformation manuelle des coins : lightView * corner (multiplication colonne-major)
    for (const auto &c : frustumCorners)
    {
        float lx = lightView.m[0] * c.x + lightView.m[4] * c.y + lightView.m[8] * c.z + lightView.m[12];
        float ly = lightView.m[1] * c.x + lightView.m[5] * c.y + lightView.m[9] * c.z + lightView.m[13];
        float lz = lightView.m[2] * c.x + lightView.m[6] * c.y + lightView.m[10] * c.z + lightView.m[14];

        minX = std::min(minX, lx);
        maxX = std::max(maxX, lx);

        minY = std::min(minY, ly);
        maxY = std::max(maxY, ly);

        minZ = std::min(minZ, lz);
        maxZ = std::max(maxZ, lz);
    }

    // --- Étape 4 : Extension de la plage Z ---
    // Les objets derrière le frustum de la caméra peuvent quand même projeter des ombres
    // dans le frustum, on étend donc minZ/maxZ par un facteur de sécurité
    float zMult = 10.0f;
    if (minZ < 0.0f)
    {
        minZ *= zMult;
    }
    else
    {
        minZ /= zMult;
    }
    if (maxZ < 0.0f)
    {
        maxZ /= zMult;
    }
    else
    {
        maxZ *= zMult;
    }

    // --- Étape 5 : Construction de la projection orthographique ---
    // Matrice ortho construite manuellement (column-major, convention OpenGL)
    Matrix4x4 lightProj;
    for (int i = 0; i < 16; i++)
    {
        lightProj.m[i] = 0.0f;
    }

    lightProj.m[0] = 2.0f / (maxX - minX);
    lightProj.m[5] = 2.0f / (maxY - minY);
    lightProj.m[10] = -2.0f / (maxZ - minZ);
    lightProj.m[12] = -(maxX + minX) / (maxX - minX);
    lightProj.m[13] = -(maxY + minY) / (maxY - minY);
    lightProj.m[14] = -(maxZ + minZ) / (maxZ - minZ);
    lightProj.m[15] = 1.0f;

    // --- Étape 6 : Textel Snapping ---
    // Sans cette étape, les ombres "nagent" (shadow swimming) lorsque la caméra se déplace
    // On projete l'origine (0,0,0) dans l'espace shadow map, on l'arrondit au textel le plus proche,
    // puis on corrige le biais dans la matrice de projection.
    Matrix4x4 cascadeVP = lightProj * lightView;

    float originX = cascadeVP.m[0] * 0.0f + cascadeVP.m[4] * 0.0f + cascadeVP.m[8] * 0.0f + cascadeVP.m[12];
    float originY = cascadeVP.m[1] * 0.0f + cascadeVP.m[5] * 0.0f + cascadeVP.m[9] * 0.0f + cascadeVP.m[13];

    float halfRes = static_cast<float>(shadowResolution) * 0.5f;
    originX *= halfRes;
    originY *= halfRes;

    float roundedX = std::round(originX);
    float roundedY = std::round(originY);

    float offsetX = (roundedX - originX) / halfRes;
    float offsetY = (roundedY - originY) / halfRes;

    lightProj.m[12] += offsetX;
    lightProj.m[13] += offsetY;

    return lightProj * lightView;
}

// ============================================================
// Execute - Passe principale de Cascaded Shadow Mapping
// ============================================================

/// @detail Étapes de la passe :
/// 1. Calcul des distances de split (practical split scheme)
/// 2. Pour chaque cascade : extraction des coins du sous-frustum et calcul de la matrice light-VP
/// 3. Conversion des distances de split en clip-space Z (pour le fragment shader)
/// 4. Attachement du GL_TEXTURE_2D_ARRAY au FBO (toutes les couches d'un coup)
/// 5. Envoi des 4 matrices au geometry shader qui route vers gl_Layer
/// 6. Rendu de tous les objets (pas de frustum culling par cascade pour simplicité)
/// @param data
void ShadowPass::Execute(RenderData &data)
{
    DirectionalLight *light = data.directionalLight;
    if (!light || !light->castsShadows || light->shadowMapArray == 0)
    {
        return;
    }
    Shader *shadowShader = data.resourceManager->GetShader("directional_shadow");
    if (!shadowShader)
    {
        return;
    }

    // --- Practical Split Scheme (Parralel-Split Shadow Maps, GPU Gems 3) ---
    // Interpole entre un split logarithmique et un split uniforme
    // Cascade split distances
    Camera *cam = data.camera;
    float nearClip = cam->GetNearPlane();
    float farClip = std::min(cam->GetFarPlane(), data.renderSettings->shadow.maxShadowDistance);
    float clipRange = farClip - nearClip;
    float lambda = data.renderSettings->shadow.cascadeSplitLambda;

    float cascadeSplits[DirectionalLight::NUM_CASCADES];
    for (int i = 0; i < DirectionalLight::NUM_CASCADES; ++i)
    {
        float p = static_cast<float>(i + 1) / static_cast<float>(DirectionalLight::NUM_CASCADES);
        float log = nearClip * std::pow(farClip / nearClip, p);
        float uniform = nearClip + clipRange * p;
        float d = lambda * log + (1.0f - lambda) * uniform;
        cascadeSplits[i] = d;
    }

    // --- Construction des 4 matrices light-space ---
    // Chaque cascade couvre [splitNear, splitFar] du frustum caméra
    Transform *lightTransform = light->GetGameObject()->GetComponent<Transform>();
    if (!lightTransform)
    {
        return;
    }
    Vector3 lightDir = lightTransform->GetWorldMatrix().GetForward();
    lightDir.Normalize();

    float aspect = static_cast<float>(data.screenWidth) / static_cast<float>(data.screenHeight);
    float fovY = cam->GetFov();
    Matrix4x4 camView = cam->GetViewMatrix();

    for (int i = 0; i < DirectionalLight::NUM_CASCADES; ++i)
    {
        float splitNear = (i == 0) ? nearClip : cascadeSplits[i - 1];
        float splitFar = cascadeSplits[i];

        std::vector<Vector3> corners = GetFrustumCornersWorldSpace(camView, fovY, aspect, splitNear, splitFar);
        data.lightSpaceMatrices[i] = ComputeCascadeMatrix(corners, lightDir, light->shadowResolution);
    }

    // --- Conversion des distances de split en clip-space Z ---
    // Le vertex Shader passe ClipSpaceZ = (proj * view * pos).z au fragment shader.
    // Pour comparer, on doit convertir nos distances (view-space positives) en clip-space :
    // clipZ = proj[10] * (-d) + proj[14]
    // Le signe négatif vient du fait que view-space Z est négatif vers l'avant en OpenGL

    Matrix4x4 camProj = Matrix4x4::CreatePerspectiveProjection(fovY, aspect, cam->GetNearPlane(), cam->GetFarPlane());

    for (int i = 0; i < DirectionalLight::NUM_CASCADES; ++i)
    {
        float clipZ = camProj.m[10] * (-cascadeSplits[i]) + camProj.m[14];
        data.cascadePlaneDistances[i] = clipZ;
    }

    // --- Rendu layered dans le FBO ---
    // glFramebufferTexture (sans suffixe "2D" attache TOUTES les layers du texture array)
    // Le geometry shader écrit gl_Layer = gl_InvocationID pour router chaque triangle vers la couche correspondance. Une seule passe de draw = 4 shadow maps.
    m_shadowFBO->Bind();

    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, light->shadowMapArray, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        m_shadowFBO->Unbind();
        return;
    }
    glDrawBuffer(GL_NONE);

    glReadBuffer(GL_NONE);
    glViewport(0, 0, light->shadowResolution, light->shadowResolution);
    glClear(GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);

    shadowShader->Use();

    // Envoi des matrices au geometry shader
    for (int i = 0; i < DirectionalLight::NUM_CASCADES; ++i)
    {
        shadowShader->SetMat4("lightSpaceMatrices[" + std::to_string(i) + "]", data.lightSpaceMatrices[i]);
    }

    // Rendu de tous les objets de la scène
    for (const auto &renderable : *data.allRenderables)
    {
        shadowShader->SetMat4("model", renderable.transform->GetWorldMatrix());
        renderable.meshRenderer->mesh->Draw();
    }

    glCullFace(GL_BACK);
    m_shadowFBO->Unbind();
}