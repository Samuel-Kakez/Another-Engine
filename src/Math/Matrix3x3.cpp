#include "Math/Matrix3x3.h"

Matrix3x3::Matrix3x3(const Matrix4x4 &mat4)
{
    m[0] = mat4.m[0];
    m[3] = mat4.m[4];
    m[6] = mat4.m[8];
    m[1] = mat4.m[1];
    m[4] = mat4.m[5];
    m[7] = mat4.m[9];
    m[2] = mat4.m[2];
    m[5] = mat4.m[6];
    m[8] = mat4.m[10];
}