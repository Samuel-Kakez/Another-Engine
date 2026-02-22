#pragma once

#include "Math/Vector3.h"
#include <vector>

// Déclaration anticipée pour éviter les inclusions croisées
struct Matrix4x4;

/**
 * @brief Représente une boîte englobante alignée sur les axes (Axis-Aligned Bounding Box)
 * @details Définit un volume par deux points, min et max
 * Utilisée pour des tests de collision et de culling rapides.
 *
 */
class AABB
{
public:
    /**
     * @brief Constructeur par défaut. Initialise une boîte "invalide".
     * @details Les valeurs min sont infinies et les valeurs max sont -infinies
     * pour que le premier point ajouté définisse la première boîte.
     *
     */
    AABB();

    /**
     * @brief Étend la boîte pour inclure un nouveau point.
     *
     * @param point le point à inclure dans le volume de la boîte
     */
    void Extend(const Vector3 &point);

    /**
     * @brief Transforme cette AABB
     *
     * @param transform
     * @return AABB
     */
    AABB Transform(const Matrix4x4 &transform) const;

    /**
     * @brief Calcule et retourne les 8 coins de la boîte englobante
     * 
     * @return std::vector<Vector3> un vecteur contenant les 8 points des coins
     */
    std::vector<Vector3> GetCorners() const;

    /**
     * @brief Vérifie si la boîte englobante intersecte une sphère
     * 
     * @param sphereCenter 
     * @param sphereRadius 
     * @return true ou false
     */
    bool IntersectsSphere(const Vector3& sphereCenter, float sphereRadius) const;

    Vector3 min; // Coin avec coordonnées minimales
    Vector3 max; // Coin avec coordonnées maximales
};