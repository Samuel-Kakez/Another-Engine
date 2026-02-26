#include "Renderer/Passes/LightingPass.h"
#include "Managers/ResourceManager.h"
#include "Managers/LightManager.h"
#include "Renderer/Shader.h"
#include "Components/MeshRenderer.h"
#include "Components/DirectionalLight.h"
#include "Core/Transform.h"
#include "Core/Scene.h"
#include "Core/GameObject.h"
#include "Components/Camera.h"
#include "Math/Vector3.h"
#include "Math/Matrix3x3.h"
#include "Debug/StatsManager.h"
#include <GLFW/glfw3.h>

void LightingPass::Execute(RenderData &data)
{
    const RenderSettings &settings = *data.renderSettings;

    // Rendu direct dans le framebuffer par défaut (écran)
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, data.screenWidth, data.screenHeight);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glDisable(GL_BLEND);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_FRAMEBUFFER_SRGB);

    Shader *lightingShader = data.resourceManager->GetShader("lighting");
    if (!lightingShader)
        return;

    lightingShader->Use();

    lightingShader->SetVec3("camPos", data.camera->GetPosition());
    lightingShader->SetFloat("exposure", settings.exposure);

    // Lumière directionnelle
    DirectionalLight *light = data.directionalLight;
    if (light)
    {
        Transform *lightTransform = light->gameObject->GetComponent<Transform>();
        const bool effectiveCastsShadows = (lightTransform != nullptr) && light->castsShadows;

        if (lightTransform)
        {
            Vector3 lightDir = lightTransform->GetWorldMatrix().GetForward();
            lightDir.Normalize();
            lightingShader->SetVec3("dirLight.direction", lightDir);
        }
        else
        {
            lightingShader->SetVec3("dirLight.direction", Vector3(0.0f, -1.0f, 0.0f));
        }
        lightingShader->SetVec3("dirLight.color", settings.lightColor);
        lightingShader->SetFloat("dirLight.intensity", settings.lightIntensity);
        lightingShader->SetBool("dirLight.castsShadows", effectiveCastsShadows);

        // --- Cascaded Shadow Map uniforms ---
        // On envoie les 4 matrices light-space et les 4 distances de plan de cascade
        // au fragment shader pour la sélection de cascade et la projection dans l'espace lumière
        for (int i = 0; i < DirectionalLight::NUM_CASCADES; ++i)
        {
            lightingShader->SetMat4(
                "lightSpaceMatrices[" + std::to_string(i) + "]", data.lightSpaceMatrices[i]);
            lightingShader->SetFloat("cascadePlaneDistances[" + std::to_string(i) + "]", data.cascadePlaneDistances[i]);
        }

        // Bind de la texture array CSM sur l'unité 8 (évite les confilts avec textures matériau)
        if (effectiveCastsShadows && light->shadowMapArray != 0)
        {
            glActiveTexture(GL_TEXTURE8);
            glBindTexture(GL_TEXTURE_2D_ARRAY, light->shadowMapArray);
            lightingShader->SetInt("shadowMapArray", 8);
        }
    }
    else
    {
        lightingShader->SetVec3("dirLight.direction", Vector3(0.0f, -1.0f, 0.0f));
        lightingShader->SetVec3("dirLight.color", Vector3(0.0f, 0.0f, 0.0f));
        lightingShader->SetFloat("dirLight.intensity", 0.0f);
        lightingShader->SetBool("dirLight.castsShadows", false);
    }

    // Ambient
    const AmbientSettings &amb = settings.ambient;
    lightingShader->SetVec3("skyColorZenith", amb.skyColorZenith);
    lightingShader->SetVec3("skyColorHorizon", amb.skyColorHorizon);
    lightingShader->SetVec3("groundColor", amb.groundColor);
    lightingShader->SetFloat("ambientIntensity", amb.intensity);

    // Rendu des objets visibles
    Material *lastMaterial = nullptr;
    for (const auto &fr : *data.cameraVisibleRenderables)
    {
        if (fr.renderable.meshRenderer->material && fr.renderable.meshRenderer->material->shader == lightingShader)
        {
            Material *currentMaterial = fr.renderable.meshRenderer->material.get();
            if (currentMaterial != lastMaterial)
            {
                currentMaterial->Bind();
                lastMaterial = currentMaterial;
                StatsManager::LogMaterialBind();
            }
            const Matrix4x4 &worldMatrix = fr.renderable.transform->GetWorldMatrix();
            lightingShader->SetMat4("model", worldMatrix);
            Matrix3x3 normalMatrix(worldMatrix.Inverse().Transpose());
            lightingShader->SetMat3("normalMatrix", normalMatrix);
            fr.renderable.meshRenderer->mesh->Draw();
        }
    }

    glBindVertexArray(0);
    glActiveTexture(GL_TEXTURE0);
    glDisable(GL_FRAMEBUFFER_SRGB);
}