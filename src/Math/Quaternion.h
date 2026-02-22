#pragma once

// Déclarations anticipées
struct Vector3;
struct Matrix4x4;

/**
 * @brief Représente une orientation dans l'espace de manière efficace et blocage de cardan.
 * @details un quaternion est composé de 4 vecteurs (x, y, z, w) et est idéal pour les calculs de rotation.
 */

struct Quaternion
{

    float x, y, z, w;

    /**
     * @brief Constructeur par défaut.
     * @details Initialise au quaternion identité (pas de rotation)
     *
     */
    Quaternion();

    /**
     * @brief Constructeur à partir de composantes
     *
     * @param x
     * @param y
     * @param z
     * @param w
     */
    Quaternion(float x, float y, float z, float w);

    /**
     * @brief Normalise le quaternion pour qu'il représente une rotation pure (longueur 1)
     *
     */
    void Normalize();

    /**
     * @brief Calcule le conjugué de ce quaternion
     * @return le quaternion conjugué.
     *
     */
    Quaternion Conjugate() const;

    /**
     * @brief Convertit le quaternion en matrice de rotation 4x4
     * @details c'est la fonction la plus importante pour l'intégration avec le système de rendu
     *
     * @return Matrix4x4 La matrice de rotation équivalente
     */
    Matrix4x4 ToRotationMatrix() const;

    /**
     * @brief Créé un quaternion à partir d'angles d'Euler (en degrés)
     *
     * @param eulerAngles Un Vector3 contenant les angles (pitch, yaw, roll) en degrés
     * @return Quaternion Le quaternion représentant cette rotation
     */
    static Quaternion FromEulerAngles(const Vector3 &eulerAngles);

    /**
     * @brief Cré un quaternion représentant une rotation autour d'un axe.
     *
     * @param axis l'axe de rotation (doit être unitaire)
     * @param angle_degrees L'angle de rotation en degrés
     * @return Quaternion Le quaternion représentant cette rotation.
     */
    static Quaternion FromAxisAngle(const Vector3 &axis, float angle_degrees);

    /**
     * @brief Calcule les angles d'Euler (pitch, yaw, roll) à partir d'un quaternion
     * @return Vector3 contenant les angles en degrés
     */
    Vector3 ToEulerAngles() const;
};

// --- Surcharges d'Opérateurs ---

/**
 * @brief Multiplie deux quaternions pour combiner leurs rotations
 * @details L'ordre est important : q2 * q1 = appliquer la rotation q1 puis q2
 * @param q1 premier quaternion
 * @param q2 deuxième quaternion
 * @return Quaternion le quaternion résultant de la composition des deux rotations
 */
Quaternion operator*(const Quaternion &q1, const Quaternion &q2);

/**
 * @brief Fait tourner un vecteur par un quaternion
 *
 * @param q le quaternion représentant la rotation
 * @param v le vecteur à faire tourner
 * @return Vector3 le nouveau vecteur après rotation
 */
Vector3 operator*(const Quaternion &q, const Vector3 &v);