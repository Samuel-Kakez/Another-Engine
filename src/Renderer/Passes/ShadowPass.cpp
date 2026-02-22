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

#include <glad/glad.h>
#include <cmath>

ShadowPass::ShadowPass()
{
    m_shadowFBO = std::make_unique<FrameBuffer>();
}

ShadowPass::~ShadowPass() = default;

void ShadowPass::Execute(RenderData &data)
{
    DirectionalLight *light = data.directionalLight;
    if (!light || !light->castsShadows || light->shadowMap == 0)
    {
        return;
    }

    Shader *shadowShader = data.resourceManager->GetShader("directional_shadow");
    if (!shadowShader)
    {
        return;
    }

    // Calcul de la matrice Light Space

    Transform *lightTransform = light->gameObject->GetComponent<Transform>();
    Vector3 lightDir = lightTransform->getWorldMatrix().GetForward();
    lightDir.Normalize();

    // Séléction du Vecteur up (évite la dégénérescence à -90°)
    Vector3 lightUp;
    if (std::abs(lightDir.y) > 0.99f)
    {
        // Lumière quasi verticale : utiliser Z comme up
        lightUp = Vector3(0.0f, 0.0f, 1.0f);
    }
    else
    {
        lightUp = Vector3(0.0f, 1.0f, 0.0f);
    }

    // Paramètres de la projection Orthographique
    float orthoSize = data.renderSettings->shadow.orthoSize;
    float nearPlane = data.renderSettings->shadow.nearPlane;
    float farPlane = data.renderSettings->shadow.farPlane;

    // Calcul de la position lumière stabilisée

    Vector3 cameraPos = data.camera->GetPosition();
    Vector3 lightPos = cameraPos - lightDir * (data.renderSettings->shadow.farPlane * 0.5f);

    int shadowRes = light->shadowResolution;
    float textelSize = (orthoSize * 2.0f) / static_cast<float>(shadowRes);

    // Construire les axes de la lumière
    Vector3 lightForward = lightDir;
    Vector3 lightRight = Cross(lightUp, lightForward);
    lightRight.Normalize();
    Vector3 lightUpOrtho = Cross(lightForward, lightRight);
    lightUpOrtho.Normalize();

    // Projeter lightPos sur les axes de la lumière
    float projX = lightPos.Dot(lightRight);
    float projY = lightPos.Dot(lightUpOrtho);
    float projZ = lightPos.Dot(lightForward);

    // Snapper sur la grille textel
    projX = std::floor(projX / textelSize) * textelSize;
    projY = std::floor(projY / textelSize) * textelSize;

    // Reconstruire la position stabilisée
    lightPos = lightRight * projX + lightUpOrtho * projY + lightForward * projZ;

    // Matrice de vue : regarde depuis lightPos dans la direction de lightDir
    Matrix4x4 lightView = Matrix4x4::CreateLookAt(
        lightPos,
        lightPos + lightDir,
        lightUp);

    // Construction manuelle de la matrice de projection orthographique vu que j'adore tout faire à la main ...
    Matrix4x4 lightProjection;
    for (int i = 0; i < 16; i++)
    {
        lightProjection.m[i] = 0.0f;
    }

    lightProjection.m[0] = 1.0f / orthoSize;                                  // scale x
    lightProjection.m[5] = 1.0f / orthoSize;                                  // scale y
    lightProjection.m[10] = -2.0f / (farPlane - nearPlane);                   // scale z
    lightProjection.m[14] = -(farPlane + nearPlane) / (farPlane - nearPlane); // translate Z
    lightProjection.m[15] = 1.0f;

    // Stocke la matrice combinée pour le shader PBR

    data.lightSpaceMatrix = lightProjection * lightView;

    // Configuration du FrameBuffer

    m_shadowFBO->Bind();
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, light->shadowMap, 0);
    glDrawBuffer(GL_NONE); // Pas de color buffer
    glReadBuffer(GL_NONE);

    glViewport(0, 0, light->shadowResolution, light->shadowResolution);
    glClear(GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);

    // Rendu de la scène

    shadowShader->Use();
    shadowShader->setMat4("lightSpaceMatrix", data.lightSpaceMatrix);

    // Rend tous les objets
    for (const auto &renderable : *data.allRenderables)
    {
        shadowShader->setMat4("model", renderable.transform->getWorldMatrix());
        renderable.meshRenderer->mesh->Draw();
    }

    // Restauration de l'état OpenGL

    glCullFace(GL_BACK);


    m_shadowFBO->Unbind();
}