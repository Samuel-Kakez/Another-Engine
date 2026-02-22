#pragma once

#include "Math/Plane.h"
#include "Math/AABB.h"
#include <array>

// Déclaration anticipée
struct Matrix4x4;

/**
 * @brief Représente un frustum de vision, défini par 6 plans.
 * @details utilisé pour le culling. Un objet est visible s'il est à l'intérieur des 6 plans de frustum
 *
 */
class Frustum
{
public:
    Frustum() = default;

    /**
     * @brief Met à jour les 6 plans du frustum à partir d'une matrice vue-projection
     *
     * @param viewProjectionMatrix La matrice combinée (Projection * vue)
     */
    void Update(const Matrix4x4& viewProjectionMatrix);

    /**
     * @brief Teste si une AABB intersecte le frustum
     *
     * @param aabb La boîte englobante à tester (en coordonnées monde)
     * @return true si la boîte est au moins partiellement à l'intérieur du frustum
     * @return false sinon
     */
    bool Intersects(const AABB &aabb) const;

    /**
     * @brief Teste si une sphère intersecte le frustum
     * 
     * @param center centre de la sphère à tester en coordonnées monde
     * @param radius rayon de la sphère à tester
     * @return true / false
     */
    bool IntersectsSphere(const Vector3& center, float radius) const;

private:
    // Les 6 plans
    // 0: gauche, 1: droite, 2: bas, 3: haut: 4: près, 5: loin
    std::array<Plane, 6> m_planes;
};