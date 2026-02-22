#pragma once
#include "Core/Component.h"
#include "Renderer/Geometry/Mesh.h"
#include "Renderer/Material.h"
#include <memory>

/**
 * @brief Un composant qui rend un GameObject visible en lui associant un Mesh
 * @details Ce composant contient une référence vers un Mesh et des propriétés de rendu
 *
 */
class MeshRenderer : public Component
{

public:
    /**
     * @brief Un pointeur vers le maillage à dessiner
     * @details Ce maillage est géré et détenu par ResourceManager
     *
     */
    Mesh *mesh;

    std::shared_ptr<Material> material;

    /**
     * @brief Constructeur
     *
     * @param mesh Le maillage à rendre
     */
    MeshRenderer(Mesh *mesh, std::shared_ptr<Material> material);

    /**
     * @brief Destructeur. Vide désormais
     *
     */
    ~MeshRenderer() override = default;
};