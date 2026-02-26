#pragma once

#include <vector>
#include <glad/glad.h> // pour les types et fonctions OpenGL

#include "Renderer/Geometry/Vertex.h"
#include "Math/AABB.h"

/**
 * @brief Représente un maillage unique et dessinable sur le GPU
 * @details Cette classe prend les données brutes de sommets et d'indices, les envoie à la VRAM
 * via des VBO/EBO, et encapsule la logique de dessin via un VAO
 *
 */
class Mesh
{
public:
    // interdiction de copie / déplacer
    Mesh(const Mesh &) = delete;
    Mesh &operator=(const Mesh &) = delete;
    Mesh(Mesh &&) = delete;
    Mesh &operator=(Mesh &&) = delete;

    /**
     * @brief Constructeur qui prend les données de maillage et configure les buffers OpenGL
     *
     * @param vertices liste des sommets à envoyer au GPU
     * @param indices liste des indices à envoyer au GPU
     */
    Mesh(const std::vector<Vertex> &vertices, const std::vector<unsigned int> &indices);

    /**
     * @brief Destructeur qui libère les buffers de la mémoire du GPU
     *
     */
    ~Mesh();

    /**
     * @brief Dessine le maillage
     * @details Active le VAO et lance l'appel au dessin indexé
     *
     */
    void Draw();

    /**
     * @brief Retourne la boîte englobante (AABB) locale du maillage
     *
     * @return const AABB&
     */
    const AABB &GetAABB() const { return aabb; }

private:
    /**
     * @brief Une fonction d'aide privée pour encapsuler la logique de la configuation complexe
     * @details créé et configure le VAO, le VBO et l'EBO
     * @param vertices
     * @param indices
     */
    void setupMesh(const std::vector<Vertex> &vertices, const std::vector<unsigned int> &indices);

    // --- identifiants des objets OpenGL ---
    // Ces unsigned int sont les identifiants uniques donnés par OpenGL pour accéder à nos objets stockés sur le GPU
    unsigned int VAO, VBO, EBO;

    // --- données du maillage ---
    // Il est utile de garder une trace du nombre d'indices pour l'appel de dessin
    unsigned int indexCount;

    // Volume local du maillage
    AABB aabb;
};