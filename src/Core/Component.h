#pragma once

// Déclaration anticipée pour briser la dépendance circulaire
// Component a besoin de connaître GameObject, et GameObject a besoin de connaître Component
class GameObject;

/**
 * @brief La classe de base pour tous les composants pouvant être attachés à un GameObject
 * @details Cette classe définit l'interface commune pour tous les composants. Elle ne doit pas être instanciée directement.
 *
 */
class Component
{
public:
    /**
     * @brief Un pointeur vers le GameObject auquel ce composant est attaché
     * @details Permet à un composant d'accéder à son propriétaire et à ses autres composants
     *
     */
    GameObject *gameObject;

    /**
     * @brief Constructeur par défaut de Component
     *
     */
    Component() : gameObject(nullptr) {}

    /**
     * @brief Destructeur virtuel de Component
     * @details Essentiel pour garantir que le destructeur de la classe dérivée (ex: Light)
     * est appelé correctement lors que le composant est détruit via son pointeur de base
     *
     */
    virtual ~Component() = default;

    /**
     * @brief Fonction de mise à jour, appelée à chaque image par le GameObject propriétaire.
     * @details Les classes dérivées peuvent surcharger cette fonction pour implémenter une logique qui s'exécute à chaque
     * image (ex : rotation, mouvement).
     * L'implémentation par défaut est vide
     *
     * @param deltaTime le temps écoulé depuis la dernière frame, en secondes
     */
    virtual void Update(float deltaTime) {}

    /**
     * @brief Fonction de mise à jour à pas de temps fixe.
     * @details Contrairement à Update(), cette fonction est appelée à une intervalle de temps constant.
     * Elle est idéale pour la physique et les logiques de jeu qui nécessitent du déterminisme.
     *
     * @param fixedDeltaTime Le temps fixe écoulé, en secondes (ex : 1/60s)
     */
    virtual void FixedUpdate(float fixedDeltaTime)
    {
    }
};