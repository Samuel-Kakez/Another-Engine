#pragma once

#include "Math/Matrix4x4.h"

/**
 * @brief Structure C++ miroir du bloc GLSL "Matrices" (layout std140).
 * @details Chaque mat4 en std140 doit être alignée à 16 octets.
 * alignas(16) force l'alignement de la structure entière.
 * Les static_asset vérifient à la compilation que le layout C++ correspond au layout GLSL.
 */
struct MatricesUBO
{
    Matrix4x4 projection;        // offset 0
    Matrix4x4 view;              // offset 64
    Matrix4x4 inverseProjection; // offset 128
    Matrix4x4 inverseView;       // offset 192
};

// Vérification à la compilation : taille totale = 256 octets
static_assert(sizeof(MatricesUBO) == 256, "MatricesUBO doit faire exactement 256 octets pour std140.");
static_assert(offsetof(MatricesUBO, view) == 64, "MatricesUBO::view doit etre a l'offset 64.");
static_assert(offsetof(MatricesUBO, inverseProjection) == 128, "MatricesUBO::inverseProjection doit etre a l'offset 128.");
static_assert(offsetof(MatricesUBO, inverseView) == 192, "MatricesUBO::inverseView doit etre a l'offset 192.");