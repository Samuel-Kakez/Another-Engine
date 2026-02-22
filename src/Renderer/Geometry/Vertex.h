#pragma once
#include "Math/Vector3.h"
#include "Math/Vector2.h"

/**
 * @brief Représente un sommet unique avec tout ses attributs
 * @details C'est la structure de données envoyée au GPU pour chaque point d'un maillage
 * L'ordre des membres (Position, Normal, TexCoords) est crucial car il doit correspondre à la disposition attendue par le Vertex Shader (layout locations)
 *
 */
struct Vertex
{

    /**
     * @brief La position du sommet dans l'espace 3D
     * (layout location 0 dans le shader)
     */
    Vector3 Position;

    /**
     * @brief La normale du sommet, utilisée pour les calculs d'éclairage
     * (layout location 1 dans le shader)
     *
     */
    Vector3 Normal;

    /**
     * @brief Les coordonnées de texture (UV du sommet)
     * (layout location 2 dans le sommet)
     */
    Vector2 TexCoords;

    Vector3 Tangent;

    Vector3 Bitangent;
};