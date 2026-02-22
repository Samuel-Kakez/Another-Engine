#pragma once

#include <string>
#include <memory>
#include "Math/Vector2.h"
#include "Math/Vector3.h"

// Déclarations anticipées
class Shader;
class Texture;

/**
 * @brief Définit les propriétés de surface d'un objet pour le rendu PBR
 * @details Contient les couleurs, les valeurs physiques (metallic, roughness)
 * et les textures qui déterminent l'apparence d'un maillage
 *
 */
class Material
{
public:
    /// @brief Pointeur non-propriétaire vers le shader utilisé pour rendre ce matériau
    Shader *shader;

    // Paramètres PBR
    /// @brief Couleur de base (diffuse) du matériau
    Vector3 albedoColor;

    /// @brief Caractère métallique de la surface
    float metallic;

    /// @brief Rugosité de la surface
    float roughness;

    /// @brief Texture d'albedo
    Texture *albedoTexture = nullptr;

    /// @brief Texture de normale
    Texture *normalTexture = nullptr;

    /// @brief Valeur de tiling
    Vector2 tiling;

    /// @brief Intensité de la normal map
    float normalMapIntensity;

    /**
     * @brief Constructeur
     *
     * @param shader pointeur non-propriétaire vers le shader à utiliser
     */
    explicit Material(Shader *shader);

    /**
     * @brief Active le shader et lie les propriétés (uniforms) du matériau
     *
     */
    void Bind() const;
};