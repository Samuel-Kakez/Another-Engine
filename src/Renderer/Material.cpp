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
    shader->setVec3("material.albedo", albedoColor);
    shader->setFloat("material.metallic", metallic);
    shader->setFloat("material.roughness", roughness);
    shader->setFloat("material.normalMapIntensity", normalMapIntensity);
    shader->setVec2("tiling", tiling);

    if (albedoTexture)
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, albedoTexture->ID);
        shader->setInt("material.albedoTexture", 0);
        shader->setBool("material.hasAlbedoTexture", true);
    }
    else
    {
        shader->setBool("material.hasAlbedoTexture", false);
    }

    if (normalTexture)
    {
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, normalTexture->ID);
        shader->setInt("material.normalTexture", 1);
        shader->setBool("material.hasNormalTexture", true);
    }
    else
    {
        shader->setBool("material.hasNormalTexture", false);
    }
}