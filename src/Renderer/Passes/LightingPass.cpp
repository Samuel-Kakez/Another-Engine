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

    lightingShader->setVec3("camPos", data.camera->GetPosition());
    lightingShader->setFloat("exposure", settings.exposure);

    // Lumière directionnelle
    DirectionalLight *light = data.directionalLight;
    if (light)
    {
        Transform *lightTransform = light->gameObject->GetComponent<Transform>();
        Vector3 lightDir = lightTransform->getWorldMatrix().GetForward();
        lightDir.Normalize();

        lightingShader->setVec3("dirLight.direction", lightDir);
        lightingShader->setVec3("dirLight.color", settings.lightColor);
        lightingShader->setFloat("dirLight.intensity", settings.lightIntensity);
        lightingShader->setBool("dirLight.castsShadows", light->castsShadows);
        lightingShader->setMat4("lightSpaceMatrix", data.lightSpaceMatrix);

        if (light->castsShadows && light->shadowMap != 0)
        {
            glActiveTexture(GL_TEXTURE8);
            glBindTexture(GL_TEXTURE_2D, light->shadowMap);
            lightingShader->setInt("shadowMap", 8);
        }
    }
    else
    {
        lightingShader->setVec3("dirLight.direction", Vector3(0.0f, -1.0f, 0.0f));
        lightingShader->setVec3("dirLight.color", Vector3(0.0f, 0.0f, 0.0f));
        lightingShader->setFloat("dirLight.intensity", 0.0f);
        lightingShader->setBool("dirLight.castsShadows", false);
    }

    // Ambient
    const AmbientSettings &amb = settings.ambient;
    lightingShader->setVec3("skyColorZenith", amb.skyColorZenith);
    lightingShader->setVec3("skyColorHorizon", amb.skyColorHorizon);
    lightingShader->setVec3("groundColor", amb.groundColor);
    lightingShader->setFloat("ambientIntensity", amb.intensity);

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
            lightingShader->setMat4("model", fr.renderable.transform->getWorldMatrix());
            fr.renderable.meshRenderer->mesh->Draw();
        }
    }

    glBindVertexArray(0);
    glActiveTexture(GL_TEXTURE0);
    glDisable(GL_FRAMEBUFFER_SRGB);
}