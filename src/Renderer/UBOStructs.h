#pragma once

#include "Math/Matrix4x4.h"

/**
 * @brief Structure C++ pour le UBO des matrices
 * L'ordre et le type des membres doivent correspondre exactement au bloc GLSL
 *
 */
struct MatricesUBO
{
    Matrix4x4 projection;
    Matrix4x4 view;
    Matrix4x4 inverseView;
    Matrix4x4 inverseProjection;
};