#pragma once

/**
 * @brief représente un vecteur ou un point dans un espace 3D
 * @details cette structure polyvalente est utilisée pour stocker des positions, rotations, échelles, normales, la couleur des lumières et l'albedo des matériaux
 *
 */

struct Vector3
{

    // membres
    float x, y, z;

    // constructeurs

    /**
     * @brief Constructeur par défaut
     * @details Initialise le vecteur à 0,0,0
     *
     */
    Vector3();

    /**
     * @brief Constructeur à partir de composantes spécifiques
     *
     * @param initialX
     * @param initialY
     * @param initialZ
     */
    Vector3(float initialX, float initialY, float initialZ);

    // fonctions membres
    /**
     * @brief Affiche les composantes du vecteur dans la console
     * @details Utile pour le débogage, affiche "Vector3(x, y, z)"
     *
     */
    void Log() const;

    /**
     * @brief calcule la longueur (magnitude) du vecteur
     *
     * @return float la longueur du vecteur
     */
    float Magnitude() const;

    /**
     * @brief modifie le vecteur pour le rendre unitaire (longueur 1)
     * @details si le vecteur est nul, il ne se passe rien, car on a une vérification dans l'implémentation
     *
     */
    void Normalize();

    // surcharge d'opérateurs

    /**
     * @brief additionne deux vecteurs
     *
     * @param other le vecteur à ajouter à celui-ci
     * @return Vector3 un nouveau vecteur résultat de l'addition composante par composante
     */
    Vector3 operator+(const Vector3 &other) const;

    /**
     * @brief soustrait un vecteur d'un autre
     *
     * @param other le vecteur à soustraire de celui-ci
     * @return Vector3 un nouveau vecteur résultat de la soustraction composante par composante
     */
    Vector3 operator-(const Vector3 &other) const;

    /**
     * @brief  multiplie le vecteur par un scalaire
     *
     * @param scalar la valeur flottante par laquelle multiplier chaque composante
     * @return Vector3 un nouveau vecteur dont chaque composante a été multipliée
     */
    Vector3 operator*(float scalar) const;

    /**
     * @brief multiplie deux vecteurs composante par composante (hadamard product)
     */
    Vector3 operator*(const Vector3 &other) const;

    float Dot(const Vector3 &other) const;
};

// fonction libre

/**
 * @brief calcule le produit vectoriel de deux vecteurs
 * @details le produit vectoriel produit un vecteur perpendiculaire aux deux vecteurs d'entrée
 * @param a // premier vecteur
 * @param b  // deuxième vecteur
 * @return Vector3 le vecteur résultant du produit vectoriel
 */
Vector3 Cross(const Vector3 &a, const Vector3 &b);