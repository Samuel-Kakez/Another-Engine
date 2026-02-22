#pragma once

#include <vector>

#include "Core/Component.h"
#include "Math/Vector3.h"
#include "Math/Matrix4x4.h"
#include "Math/Quaternion.h"

/**
 * @brief Un composant qui stocke la position, la rotation et l'échelle d'un GameObject
 * @details c'est un composant fondamental que presque tous les GameObjects posséderont. Il gère désormais les relations
 * parent-enfant pour former une hiérarchie de scène (ou graphe de scène). Les transformations (position, orientation, scale)
 * sont maintenant relatives au Transform parent.
 */
class Transform : public Component
{
public:
    /**
     * @brief Constructeur par défaut pour Transform
     * @details Initialise la position et la rotation à (0,0,0), et l'échelle à (1,1,1) (transformation neutre)
     *
     */
    Transform();

    /**
     * @brief Destructeur. S'assure de notifier son parent qu'il est détruit pour maintenir la hiérarchie cohérente.
     *
     */
    ~Transform();

    // --- Getters ---
    const Vector3 &GetPosition() const { return m_position; }
    const Quaternion &GetOrientation() const { return m_orientation; }
    const Vector3 &GetScale() const { return m_scale; }

    // --- Setters ---
    void SetPosition(const Vector3 &position);
    void SetOrientation(const Quaternion &orientation);
    void SetScale(const Vector3 &scale);

    /**
     * @brief Calcule et retourne la matrice de transformation locale de l'objet.
     * @details Combine échelle, rotation et translation locales en une matrice 4x4
     * Cette matrice représente la transformation de l'objet par rapport à son parent.
     * @return Matrix4x4 La matrice locale combinée
     */
    Matrix4x4 getLocalMatrix() const;

    /**
     * @brief Calcule et retourne la matrice modèle (Model Matrix) de l'objet dans l'espace Monde.
     * @details Combine la matrice de son parent avec sa propre matrice locale de manière récursive
     * C'est cette matrice qu'on envoie au shader pour le rendu. On retourne une ref constante désormais
     * @return Matrix4x4 La matrice monde combinée
     */
    const Matrix4x4& getWorldMatrix() const;

    /**
     * @brief Définit le parent de ce Transform
     * @details Attache ce transform à un autre dans la hiérarchie. Passer "nullptr" détache le transform
     * et en fait un objet racine de la scène.
     *
     * @param newParent Pointeur vers le nouveau transform parent.
     */
    void SetParent(Transform *newParent);

    /**
     * @brief Récupère le parent de ce transform.
     *
     * @return Transform* Le pointeur vers le parent, ou nullptr si c'est un objet racine.
     */
    Transform *GetParent() const { return m_parent; }

    /**
     * @brief Récupère la liste des enfants de ce transform
     *
     * @return const std::vector<Transform *>& Une référence constante vers la liste des enfants.
     */
    const std::vector<Transform *> &GetChildren() const { return m_children; }

    Vector3 getWorldPosition() const;

private:
    // --- Données de transformation locale (maintenant privées) ---
    Vector3 m_position;
    Quaternion m_orientation;
    Vector3 m_scale;

    // --- Cache pour la matrice World ---
    mutable Matrix4x4 m_cachedWorldMatrix;
    mutable bool m_isDirty = true; // Dirty = cache plus à jour

    /**
     * @brief Invalide le cache de la matrice world et propage l'invalidation aux enfants
     * @details Appelé chaque fois que la transformation locale change
     *
     */
    void InvalidateWorldMatrix();

    /**
     * @brief Ajoute un enfant à ce transform. Usage interne.
     *
     * @param child le transform enfant à ajouter
     */
    void AddChild(Transform *child);

    /**
     * @brief Retire un enfant de ce transform. Usage interne.
     *
     * @param child le transform enfant à retirer
     */
    void RemoveChild(Transform *child);

    // --- Relations hiérarchiques ---
    // Note : ce sont des pointeurs bruts non-propriétaires. La scene possède toujours les GameObjects
    // et leurs transforms via des unique_ptr. On fait que les référencer ici
    Transform *m_parent = nullptr;
    std::vector<Transform *> m_children;
};