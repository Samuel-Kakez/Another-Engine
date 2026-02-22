#pragma once
#include "Math/Vector3.h"

/**
 * @brief Représente un plan infini dans l'espace 3D
 * @details Défini par l'équation Ax + By + Cz + D = 0
 * La normale (A B C) est stockée dans 'normal' et D dans 'distance'
 *
 */
class Plane
{
public:
    Plane() : distance(0.0f) {}

    Plane(const Vector3 &normal, float distance) : normal(normal), distance(distance) {}

    /**
     * @brief Calcule la distance signée d'un point au plan
     * @details le signe indique de quel côté du plan se trouve le point
     * > 0 côté positif (direction de la normale)
     * = 0 sur le plan
     * < 0 côté négatif
     * @param point Le point à tester
     * @return float La distance signée
     */
    float GetSignedDistanceToPoint(const Vector3 &point) const;

    Vector3 normal; // La normale du plan
    float distance; // La distance D de l'origine au plan le long de sa normale
};