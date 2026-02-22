#pragma once

#include "Matrix4x4.h"

/**
 * @brief Représente une matrice 3x3 en ordre colonne-majeur.
 * @details Utilisée pour les transformations de direction (comme les normales) en 3D, en ignorant la translation
 *
 */
struct Matrix3x3
{
    /**
     * @brief Les 9 éléments de la matrice stockés dans un tableau
     *
     */
    float m[9];

    /**
     * @brief Construit une matrice 3x3 à partir de la partie supérieure gauche d'une matrice 4x4
     *
     * @param mat4 La matrice 4x4 source
     */
    Matrix3x3(const Matrix4x4 &mat4);
};