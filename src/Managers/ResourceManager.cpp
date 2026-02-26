#include "Renderer/Texture.h"
#include "Managers/ResourceManager.h"
#include "Debug/Logger.h"

#include <stb_image.h>
#include <memory>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

// Définitions pour filtrage anisotrope
#ifndef GL_TEXTURE_MAX_ANISOTROPY_EXT
#define GL_TEXTURE_MAX_ANISOTROPY_EXT 0x84FE
#endif

#ifndef GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT
#define GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT 0x84FF
#endif

Shader *ResourceManager::GetShader(const std::string &name, const std::string &vShaderFile, const std::string &fShaderFile, const std::string &gShaderFile)
{
    // On cherche d'abord le shader par son nom.
    auto it = m_Shaders.find(name);
    if (it != m_Shaders.end())
    {
        // si on le trouve, on retourne le pointeur brut qu'elle contient (.get())
        return it->second.get();
    }

    // Non trouvé: on essaye de le créer
    if (!vShaderFile.empty() && !fShaderFile.empty())
    {
        // Logique de création avec ou sans geometry shader
        std::unique_ptr<Shader> newShader;

        if (!gShaderFile.empty())
        {
            // avec geometry shader
            newShader = std::make_unique<Shader>(vShaderFile.c_str(), fShaderFile.c_str(), gShaderFile.c_str());
        }
        else
        {
            // sans geometry shader
            newShader = std::make_unique<Shader>(vShaderFile.c_str(), fShaderFile.c_str());
        }
        if (!newShader || !newShader->IsValid())
        {
            LOG_ERROR("shader '%s' invalide, non chargé", name.c_str());
            return nullptr;
        }

        Shader *ptr = newShader.get(); // On garde le pointeur avant le std::move
        m_Shaders[name] = std::move(newShader);
        LOG_INFO("shader '%s' chargé.", name.c_str());
        return ptr;
    }

    // Non trouvé et pas de chemin fourni
    LOG_WARN("shader '%s' demandé mais introuvable et aucun chemin fourni.", name.c_str());
    return nullptr;
}

Model *ResourceManager::GetModel(const std::string &path)
{
    // Si le modèle n'est pas déjà dans notre cache...
    if (m_Models.find(path) == m_Models.end())
    {
        // On le charge et on le déplace (avec move)
        auto newModel = std::make_unique<Model>(path);
        m_Models[path] = std::move(newModel);
        LOG_INFO("modèle '%s' chargé (%zu meshes).", path.c_str(), m_Models[path]->meshes.size());
    }
    // on retourne le pointeur du modèle
    return m_Models.at(path).get();
}

Texture *ResourceManager::GetTexture(const std::string &name, const std::string &path, bool srgb)
{
    // chercher d'abord la texture
    auto it = m_Textures.find(name);
    if (it != m_Textures.end())
    {
        return it->second.get();
    }

    // non trouvé : on la charge si le chemin est fourni
    if (path.empty())
        return nullptr;

    auto newTexture = std::make_unique<Texture>();
    int nrChannels;

    // Inverse la texture verticalement
    stbi_set_flip_vertically_on_load(true);

    // stbi charge l'image
    unsigned char *data = stbi_load(path.c_str(), &newTexture->Width, &newTexture->Height, &nrChannels, 0);
    if (data)
    {
        GLenum internalFormat = 0;
        GLenum dataFormat = 0;
        if (nrChannels == 1)
        {
            internalFormat = GL_RED;
            dataFormat = GL_RED;
        }
        else if (nrChannels == 2)
        {
            internalFormat = GL_RG;
            dataFormat = GL_RG;
        }
        else if (nrChannels == 3)
        {
            internalFormat = srgb ? GL_SRGB : GL_RGB;
            dataFormat = GL_RGB;
        }
        else if (nrChannels == 4)
        {
            internalFormat = srgb ? GL_SRGB_ALPHA : GL_RGBA;
            dataFormat = GL_RGBA;
        }
        else
        {
            LOG_ERROR("format texture non supporté (%d canaux) pour '%s'", nrChannels, path.c_str());
            stbi_image_free(data);
            return nullptr;
        }

        glGenTextures(1, &newTexture->ID);
        glBindTexture(GL_TEXTURE_2D, newTexture->ID);
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, newTexture->Width, newTexture->Height, 0, dataFormat, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        // Options de la texture
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // Gestion du filtrage anisotrope
        if (glfwExtensionSupported("GL_EXT_texture_filter_anisotropic"))
        {
            GLfloat maxAnisotropy = 1.0f;
            glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropy);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAnisotropy);
        }
        LOG_INFO("texture '%s' chargée (%dx%d, %d canaux).", path.c_str(), newTexture->Width, newTexture->Height, nrChannels);
    }
    else
    {
        LOG_ERROR("échec du chargement de la texture '%s'.", path.c_str());
        stbi_image_free(data);
        return nullptr; // On ne cache pas une texture en échec
    }
    stbi_image_free(data); // Libère la mémoire de l'image

    Texture *ptr = newTexture.get();
    m_Textures[name] = std::move(newTexture);
    return ptr;
}

std::shared_ptr<Material> ResourceManager::GetOrCreateMaterial(const std::string &name, Shader *shader)
{
    // On cherche le matériau par son nom
    auto it = m_Materials.find(name);
    if (it != m_Materials.end())
    {
        // s'il est trouvé, on retourne le pointeur partagé existant
        return it->second;
    }
    // si pas trouvé, on le créé, le stocke et le retourne
    auto newMaterial = std::make_shared<Material>(shader);
    m_Materials[name] = newMaterial;
    LOG_INFO("matériau '%s' créé.", name.c_str());
    return newMaterial;
}

std::shared_ptr<Material> ResourceManager::GetMaterial(const std::string &name)
{
    auto it = m_Materials.find(name);
    if (it != m_Materials.end())
    {
        return it->second;
    }
    return nullptr;
}

void ResourceManager::Clear()
{
    // cette fonction est maintenant beaucoup plus simple.
    // L'appel à .clear() surla map détruit tous les éléments qu'elle contient.
    // En détruisant chaque std::unique_ptr, le destructeur de ce dernier est appelé, ce qui libère la mémoire automatiquement.
    // plus besoin de boucler et de delete manuellement.
    m_Materials.clear();
    LOG_INFO("matériaux libérés.");
    m_Shaders.clear(); // Vide la carte des shaders
    LOG_INFO("shaders libérés.");
    m_Models.clear(); // vide la carte des modèles
    LOG_INFO("models libérés.");
    m_Textures.clear();
    LOG_INFO("textures libérées.");
    LOG_INFO("toutes les ressources ont été libérées.");
}
