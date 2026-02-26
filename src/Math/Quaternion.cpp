#include "Math/Quaternion.h"
#include "Math/Vector3.h"
#include "Math/Matrix4x4.h"
#include "Math/Constants.h"
#include <cmath>

// Conversion de degrés en radians, locale à ce fichier

static float toRadians(float degrees)
{
    return degrees * (Math::PI / 180.0f);
}

Quaternion::Quaternion() : x(0.0f), y(0.0f), z(0.0f), w(1.0f)
{
}

Quaternion::Quaternion(float x, float y, float z, float w) : x(x), y(y), z(z), w(w)
{
}

void Quaternion::Normalize()
{
    float mag = sqrtf(x * x + y * y + z * z + w * w);
    if (mag > 0.0f)
    {
        x /= mag;
        y /= mag;
        z /= mag;
        w /= mag;
    }
}

Quaternion Quaternion::Conjugate() const
{
    return Quaternion(-x, -y, -z, w);
}

Matrix4x4 Quaternion::ToRotationMatrix() const
{

    // Crée une matrice identité sur laquelle on va appliquer la rotation
    Matrix4x4 result;

    // Pré-calcul des carrés et des produits croisés pour l'efficacité.
    float x2 = x * x;
    float y2 = y * y;
    float z2 = z * z;

    float xy = x * y;
    float xz = x * z;
    float yz = y * z;

    float wx = w * x;
    float wy = w * y;
    float wz = w * z;

    // Calcule les éléments de la matrice de rotation 3x3
    // Cette formule mathématique standard convertit l'axe et l'angle du quaternion en une base orthonormée (les nouveaux axes X Y Z)

    // Colonne 1 (nouveau X)
    result.m[0] = 1.0f - 2.0f * (y2 + z2);
    result.m[1] = 2.0f * (xy + wz);
    result.m[2] = 2.0f * (xz - wy);

    // Colonne 2 (nouveau Y)
    result.m[4] = 2.0f * (xy - wz);
    result.m[5] = 1.0f - 2.0f * (x2 + z2);
    result.m[6] = 2.0f * (yz + wx);

    // Colonne 3 (nouveau Z)
    result.m[8] = 2.0f * (xz + wy);
    result.m[9] = 2.0f * (yz - wx);
    result.m[10] = 1.0f - 2.0f * (x2 + y2);

    return result;
}

Quaternion Quaternion::FromEulerAngles(const Vector3 &eulerAngles)
{
    // Convertit les angles d'Euler (en degrés) pour chaque axe principal en quaternions individuels
    // On prend l'ordre Yaw (Y), Pitch (X) et Roll (Z)

    Quaternion qYaw = Quaternion::FromAxisAngle({0.0f, 1.0f, 0.0f}, eulerAngles.y);   // Lacet
    Quaternion qPitch = Quaternion::FromAxisAngle({1.0f, 0.0f, 0.0f}, eulerAngles.x); // Tangage
    Quaternion qRoll = Quaternion::FromAxisAngle({0.0f, 0.0f, 1.0f}, eulerAngles.z);  // Roulis

    // combine les rotations en les multipliant. L'ordre est inversé comme les matrices
    return qYaw * qPitch * qRoll;
}

Quaternion Quaternion::FromAxisAngle(const Vector3 &axis, float angle_degrees)
{
    // la conversion d'un axe-angle en quaternion utilise la moitié de l'angle
    float halfAngle_rad = toRadians(angle_degrees) / 2.0f;
    float s = sin(halfAngle_rad);

    Quaternion q;
    // La partie "w" (scalaire) représente le cosinus de la motié de l'angle
    q.w = cos(halfAngle_rad);
    // La partie "xyz" (vectorielle) est l'axe de rotation, mis à l'échelle par le sinus de la moitié de l'angle
    q.x = axis.x * s;
    q.y = axis.y * s;
    q.z = axis.z * s;
    return q;
}

Vector3 Quaternion::ToEulerAngles() const
{
    Vector3 angles;

    // Tangage (pitch, rotation X)
    float sinp = 2.0f * (w * x + y * z);
    if (std::abs(sinp) >= 1.0f)
    {
        angles.x = std::copysign(Math::PI / 2.0f, sinp);
    }
    else
    {
        angles.x = std::asin(sinp);
    }

    // Lacet (yaw, rotation Y)
    float siny = 2.0f * (w * y - x * z);
    float cosy = 1.0f - 2.0f * (x * x + y * y);
    angles.y = std::atan2(siny, cosy);

    // Roulis (roll, rotation Z)
    float sinr = 2.0f * (w * z - x * y);
    float cosr = 1.0f - 2.0f * (x * x + z * z);
    angles.z = std::atan2(sinr, cosr);

    // Conversion de radians en degrés
    return angles * (180.0f / Math::PI);
}

// Opérateur de multiplication de deux quaternions (Produit de Hamilton)
Quaternion operator*(const Quaternion &q1, const Quaternion &q2)
{
    Quaternion result;

    // La multiplication de quaternions combine leurs rotations respectives.
    // La formule est dérivée des règles de multiplication des nombres complexes étendus.

    result.w = q1.w * q2.w - q1.x * q2.x - q1.y * q2.y - q1.z * q2.z; // partie scalaire
    result.x = q1.w * q2.x + q1.x * q2.w + q1.y * q2.z - q1.z * q2.y; // partie vectorielle x
    result.y = q1.w * q2.y - q1.x * q2.z + q1.y * q2.w + q1.z * q2.x; // partie vectorielle y
    result.z = q1.w * q2.z + q1.x * q2.y - q1.y * q2.x + q1.z * q2.w; // partie vectorielle z

    return result;
}

// Opérateur pour faire tourner un vecteur par un quaternion
Vector3 operator*(const Quaternion &q, const Vector3 &v)
{
    // La formule mathématique pour faire tourner un vecteur v par un quaternion q est : v' = q * v * q_conjugué
    // pour cela, on doit temporairement représenter le vecteur v comme un quaternion "pur" avec w=0
    Quaternion v_quat(v.x, v.y, v.z, 0.0f);

    // On applique la formule
    Quaternion result_quat = q * v_quat * q.Conjugate();

    // Le résultat est un autre quaternion pur. On extrait juste sa partie vectorielle
    return Vector3(result_quat.x, result_quat.y, result_quat.z);
}