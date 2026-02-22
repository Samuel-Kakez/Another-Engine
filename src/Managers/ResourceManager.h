#pragma once

#include <map>
#include <string>
#include <vector>
#include <memory>

#include "Renderer/Shader.h"
#include "Renderer/Geometry/Mesh.h"
#include "Renderer/Geometry/Vertex.h"
#include "Renderer/Geometry/Model.h"
#include "Renderer/Texture.h"
#include "Renderer/Material.h"
/**
 * @brief Classe qui gère le chargement, la génération et le stockage des ressources.
 * @details Fournit un accès centralisé aux shaders et aux maillages.
 * Une instance de cette classe est détenue par l'Engine
 *
 */
class ResourceManager
{
public:
    // On peut maintenant créer un objet ResourceManager
    ResourceManager() = default;

    /**
     * @brief Récupère un shader depuis le cache ou le charge depuis des fichiers.
     *
     * @param name le nom unique du shader
     * @param vShaderFile le chemin vers le chemin du vertex shader (si pas encore en cache)
     * @param fShaderFile le chemin vers le chemin du fragment shader (si pas encore en cache)
     * @param gShaderFile le chemin vers le du geometry shader (optionnel, si pas encore en cache)
     * @return Shader* un pointeur vers l'objet shader
     */
    Shader *GetShader(const std::string &name, const std::string &vShaderFile = "", const std::string &fShaderFile = "", const std::string& gShaderFile = "");

    /**
     * @brief Charger un modèle depuis un fichier et le met en cache
     * @details Si le modèle est déjà en cache, on retourne juste le pointeur
     * @param path chemin vers le fichier du modèle
     * @return Model* pointeur vers le modèle
     */
    Model *GetModel(const std::string &path);

    /**
     * @brief Récupère une texture depuis le cache ou la charge depuis un fichier
     *
     * @param name nom unique de la texture
     * @param path chemin vers le fichier de l'image
     * @param srgb indique s'il faut gérer la correction gamma
     * @return Texture* un pointeur vers l'objet texture
     */
    Texture *GetTexture(const std::string &name, const std::string &path = "", bool srgb = false);

    std::shared_ptr<Material> GetOrCreateMaterial(const std::string& name, Shader* shader);
    std::shared_ptr<Material> GetMaterial(const std::string& name);

    /**
     * @brief libère toutes les ressources stockées
     * @details doit être appelée à la fin du programme pour éviter toute fuite de mémoire
     */
    void Clear();

private:
    // Les maps ne sont plus statiques. Chaque instance de ResourceManager aura ses propres ressources
    std::map<std::string, std::unique_ptr<Shader>> m_Shaders;
    std::map<std::string, std::unique_ptr<Model>> m_Models;
    std::map<std::string, std::unique_ptr<Texture>> m_Textures;
    std::map<std::string, std::shared_ptr<Material>> m_Materials;
};