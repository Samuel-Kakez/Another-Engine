#pragma once

/**
 * @brief Représente un vecteur ou un point dans un espace 2D
 * @details Cette structure est principalement utilisée pour stocker des coordonnées de textures
 */

struct Vector2
{

    /**
     * @brief composante horizontale du vecteur
     *
     */
    float x;

    /**
     * @brief composante verticale du vecteur
     *
     */
    float y;

    /**
     * @brief Constructeur par défaut
     * @details Initialise le vecteur à x0.0 et y0.0
     *
     */
    Vector2();

    /**
     * @brief Constructeur à partir de composantes spécifiques
     * @details créé un vecteur avec valeurs x et y fournies
     * @param initialX
     * @param initialY
     */
    Vector2(float initialX, float initialY);
};