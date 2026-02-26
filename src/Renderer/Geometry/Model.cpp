#include "Renderer/Geometry/Model.h"
#include "Renderer/Geometry/Vertex.h"
#include "Math/Vector2.h"
#include "Math/Vector3.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Debug/Logger.h"

// Le constructeur appelle simplement la fonction de chargement principale
Model::Model(const std::string &path)
{
    loadModel(path);
}

void Model::loadModel(const std::string &path)
{
    Assimp::Importer importer;
    // Lire le fichier aiProcessTriangulate s'assure que toutes les faces sont des triangles
    const aiScene *scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace);

    // vérifier les erreurs de chargement
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        LOG_ERROR("échec du chargement Assimp - %s", importer.GetErrorString());
        return;
    }
    // Commencer le traitement récursif des noeuds de la scène, en partant du noeud racine
    processNode(scene->mRootNode, scene);
}

void Model::processNode(aiNode *node, const aiScene *scene)
{
    // traiter tous les maillages du noeud actuel
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(processMesh(mesh, scene));
    }

    // puis, traiter récursivement tous les enfants de ce noeud
    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        processNode(node->mChildren[i], scene);
    }
}

std::unique_ptr<Mesh> Model::processMesh(aiMesh *mesh, const aiScene *scene)
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    // Parcourir tous les sommets du maillage
    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        Vertex vertex{};

        // position
        vertex.Position = {mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z};

        // normales
        if (mesh->HasNormals())
        {
            vertex.Normal = {mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z};
        }

        // coordonnées de textures
        if (mesh->mTextureCoords[0])
        {
            vertex.TexCoords = {mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y};
        }
        else
        {
            vertex.TexCoords = Vector2(0.0f, 0.0f);
        }

        if (mesh->HasTangentsAndBitangents())
        {
            vertex.Tangent = {mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z};
            vertex.Bitangent = {mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z};
        }

        vertices.push_back(vertex);
    }

    // parcourir toutes les faces (triangles) pour récupérer les indices des sommets
    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++)
        {
            indices.push_back(face.mIndices[j]);
        }
    }

    // créer notre mesh avec les données extraites
    return std::make_unique<Mesh>(vertices, indices);
}