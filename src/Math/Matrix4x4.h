#pragma once

// déclaration anticipée pour éviter les inclusions circulaires
// Vector3 a besoin de Matrix4x4 et vice versa
struct Vector3;

/**
 * @brief Représente une matrice 4x4 en ordre colonne-majeur
 * @details Principalement utilisée pour les transformations 3D (translation, rotation, échelle)
 * et les projections (perspective, orthographique)
 */
struct Matrix4x4
{
    /**
     * @brief Les 16 éléments de la matrice stockés dans un tableau
     *
     */
    float m[16];

    /**
     * @brief Constructeur par défaut
     * @details Initialise la matrice à la matrice identité. La matrice identité représente une transformation qui ne fait rien. (Ma matrice * Mat Identité = Ma matrice)
     *
     */
    Matrix4x4();

    /**
     * @brief Affiche le contenu de la matrice dans la console pour débogage
     *
     */
    void Log() const;

    // --- Fonctions de Création Statiques (Factory Methods) ---

    /**
     * @brief Créé une matrice de translation
     * @details Une matrice de translation déplace un objet (un "modèle") sans changer son orientation ou taille
     * Si on applique cette matrice à un point, il sera déplacé de la distance et dans la direction indiquée par le vecteur de translation
     *
     * @param translation le vecteur indiquant de combien déplacer l'objet sur les axes x y z
     * @return une matrice de transformation qui ne fait que translater
     */
    static Matrix4x4 CreateTranslation(const Vector3 &translation);

    /**
     * @brief Créé une matrice de mise à l'échelle
     * @details Une matrice de mise à l'échelle change la taille d'un objet sur x y z
     * Une échelle de (1 1 1) ne change rien. (2 1 1) le rend 2x plus large
     *
     * @param scale le vecteur indiquant le facteur de mise à l'échelle pour chaque axe
     * @return Une matrice de transformation qui ne fait que mettre à l'échelle
     */
    static Matrix4x4 CreateScale(const Vector3 &scale);

    /**
     * @brief Créé une matrice de rotation autour de l'axe X
     *
     * @param angle_radians l'angle de la rotation en radians
     * @return Une matrice de transformation qui effectue une rotation autour de l'axe X
     */
    static Matrix4x4 CreateRotationX(float angle_radians);

    /**
     * @brief Créé une matrice de rotation autour de l'axe Y
     *
     * @param angle_radians l'angle de la rotation en radians
     * @return Une matrice de transformation qui effectue une rotation autour de l'axe Y
     */
    static Matrix4x4 CreateRotationY(float angle_radians);

    /**
     * @brief Créé une matrice de rotation autour de l'axe Z
     *
     * @param angle_radians l'angle de la rotation en radians
     * @return Une matrice de transformation qui effectue une rotation autour de l'axe Z
     */
    static Matrix4x4 CreateRotationZ(float angle_radians);

    /**
     * @brief Créé une matrice de vue (View Matrix)
     * @details C'est l'une des matrices les plus importantes. Elle ne transforme pas un objet, mais plutôt le monde entier pour le positionner par rapport à la caméra.
     * C'est l'équivalent de placer et d'orienter la caméra dans la scène
     *
     * @param eye la position de la caméra dans le monde
     * @param target le point que la caméra regarde
     * @param up le vecteur qui indique la direction du "haut" pour la caméra (0 1 0)
     * @return une matrice de vue prête à petre utilisée dans le rendu
     */
    static Matrix4x4 CreateLookAt(const Vector3 &eye, const Vector3 &target, const Vector3 &up);

    /**
     * @brief Créé une matrice de projection perspective.
     * @details C'est la deuxième matrice spéciale du rendu. Elle simule la perspective : les objets plus éloignés apparaissent plus petits.
     * Elle définit un volume de vue en forme de pyramide tronquée (un "frustum"). Tout ce qui est à l'intérieur de ce volume sera visible à l'écran
     *
     * @param fov_y_degrees le champ de vision vertical (fov) en degrés
     * @param aspect_ratio le rapport longueur/hauteur de la fenêtre d'affichage
     * @param near_plane la distance de coupe proche
     * @param far_plane la distance de coupe éloigné
     * @return une matrice de projection perspective
     */
    static Matrix4x4 CreatePerspectiveProjection(float fov_y_degrees, float aspect_ratio, float near_plane, float far_plane);

    /**
     * @brief Calcule l'inverse d'une matrice de transformation rigide (rotation + translation)
     * @details cette méthode est optimisée et ne fonctionne que pour les matrices qui ne contiennent pas
     * de mise à l'échelle non-uniforme ou de cisaillement, parfait pour une view matrix
     *
     * @return Matrix4x4 la matrice inversée.
     */
    Matrix4x4 InverseRigid() const;

    /**
     * @brief Calcule l'inverse d'une matrice 4x4 générale
     * @details Cette méthode est plus coûteuse mais fonctionne avec toutes les transformations, y compris la mise à l'échelle
     * Indispensable pour calculer la matrice des normales correctement
     * @return Matrix4x4 la matrice inversée
     */
    Matrix4x4 Inverse() const;

    /**
     * @brief Transpose la matrice
     * @details Échange les lignes et les colonnes de la matrice
     * @return Matrix4x4 la matrice transposée
     */
    Matrix4x4 Transpose() const;

    /**
     * @brief Transforme un vecteur directionnel par la matrice
     * @details applique uniquement la rotation et l'échelle (partie 3x3) au vecteur.
     * @param dir vecteur directionnel à transformer
     * @return Vector3 vecteur directionnel transformé
     */
    Vector3 TransformDirection(const Vector3 &dir) const;

    /**
     * @brief Transform un point (W = 1) avec division perspective
     * @details Multiplie [x, y, z, 1] par la matrice, puis divise par W.
     * Indispensable pour les reprojections NDC -> world (matrice VP inverse).
     */
    Vector3 TransformPoint(const Vector3 &point) const;

    // --- Surcharge d'opérateurs ---
    /**
     * @brief Multiplie deux matrices
     * @details Permet de combiner des transformations (ex: échelle PUIS rotation).
     * @param other la matrice à multiplier (à droite)
     * @return une nouvelle matrice représentant la composition des deux transformations
     *
     */
    Matrix4x4 operator*(const Matrix4x4 &other) const;

    /**
     * @brief Transforme un vecteur 3D pour la matrice.
     * @details Applique la transformation représentée par la matrice à un point ou un vecteur.
     * @param vector le vecteur à transformer
     * @return un nouveau vecteur qui est la version transformée du vecteur d'origine
     *
     */
    Vector3 operator*(const Vector3 &vector) const;

    /**
     * @brief Getter pour la translation
     *
     * @return Vector3
     */
    Vector3 GetTranslation() const;

    /**
     * @brief Getter pour l'avant
     *
     * @return Vector3
     */
    Vector3 GetForward() const;
};