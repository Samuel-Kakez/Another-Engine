#include "Renderer/Material.h"
#include "Renderer/Shader.h"
#include "Renderer/Texture.h"
#include <glad/glad.h>

Material::Material(Shader *shader)
    : shader(shader),
      albedoColor(1.0f, 1.0f, 1.0f),
      metallic(0.0f),
      roughness(0.5f),
      tiling(1.0f, 1.0f),
      normalMapIntensity(1.0f)
{
}

void Material::Bind() const
{
    if (!shader)
        return;

    shader->Use();

    // Liaison des valeurs PBR
    shader->SetVec3("material.albedo", albedoColor);
    shader->SetFloat("material.metallic", metallic);
    shader->SetFloat("material.roughness", roughness);
    shader->SetFloat("material.normalMapIntensity", normalMapIntensity);
    shader->SetVec2("tiling", tiling);

    if (albedoTexture)
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, albedoTexture->ID);
        shader->SetInt("material.albedoTexture", 0);
        shader->SetBool("material.hasAlbedoTexture", true);
    }
    else
    {
        shader->SetBool("material.hasAlbedoTexture", false);
    }

    if (normalTexture)
    {
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, normalTexture->ID);
        shader->SetInt("material.normalTexture", 1);
        shader->SetBool("material.hasNormalTexture", true);
    }
    else
    {
        shader->SetBool("material.hasNormalTexture", false);
    }
}