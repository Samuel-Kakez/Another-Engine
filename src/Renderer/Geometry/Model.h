#pragma once

#include <vector>
#include <string>
#include <memory>

#include "Renderer/Geometry/Mesh.h"

// Déclarations anticipées des types Assimp pour éviter d'inclure les en-têtes assimp
struct aiNode;
struct aiScene;
struct aiMesh;

/**
 * @brief Représente un modèle 3D complet chargé depuis un fichier.
 * @details Un modèle est une collection d'un ou plusieurs maillages (Mesh) et agit comme un
 * conteneur pour la géométrie.
 *
 */
class Model
{
public:
    /**
     * @brief Construit un objet Model en chargeant les données depuis un fichier.
     *
     * @param path le chemin vers le fichier du modèle
     */
    Model(const std::string &path);
    /// @brief Liste des maillages uniques qui composent ce modèle
    std::vector<std::unique_ptr<Mesh>> meshes;

private:
    /**
     * @brief Charge le modèle à partir du chemin spécifié en utilisant Assimp.
     *
     * @param path le chemin vers le fichier du modèle
     */
    void loadModel(const std::string &path);

    /**
     * @brief Traite récursivement un noeud de la scène Assimp.
     * @details extrait les maillages de chaque noeud et de ses enfants
     *
     * @param node le noeud de la scène assimp à traiter
     * @param scene la scène assimp complète, qui contient les données des maillages
     */
    void processNode(aiNode *node, const aiScene *scene);

    /**
     * @brief Convertit un maillage Assimp (aiMesh) en notre format de Mesh
     *
     * @param mesh le maillage à convertir
     * @param scene la scène Assimp (pas encore utilisée)
     * @return std::unique_ptr<Mesh> vers le nouvel objet Mesh créé
     */
    std::unique_ptr<Mesh> processMesh(aiMesh *mesh, const aiScene *scene);
};