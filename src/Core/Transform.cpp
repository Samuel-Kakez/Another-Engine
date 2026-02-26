#include <algorithm>

#include "Core/Transform.h"
#include "Core/ComponentFactory.h"
#include "Core/GameObject.h"
#include <nlohmann/json.hpp>

// le constructeur utilise la liste d'initialisation
Transform::Transform() : m_position(Vector3(0.0f, 0.0f, 0.0f)),
                         m_orientation(),
                         m_scale(Vector3(1.0f, 1.0f, 1.0f)),
                         m_parent(nullptr),
                         m_isDirty(true) // cache initialement invalide
{
}

Transform::~Transform()
{
    // Avant de mourir, je dois me détacher de mon parent pour ne pas laisser un pointeur invalide.
    if (m_parent)
    {
        m_parent->RemoveChild(this);
    }

    // Je dois aussi informer mes enfants qu'ils n'ont plus de parent.
    // C'est une sécurité, bien que la destruction en cascade de la scène gère cela.
    for (Transform *child : m_children)
    {
        child->m_parent = nullptr;
        child->InvalidateWorldMatrix(); // Les enfants doivent recalculer leur matrice
    }
}

// --- Nouveaux Setters ---

void Transform::SetPosition(const Vector3 &position)
{
    m_position = position;
    InvalidateWorldMatrix();
}

void Transform::SetOrientation(const Quaternion &orientation)
{
    m_orientation = orientation;
    InvalidateWorldMatrix();
}

void Transform::SetScale(const Vector3 &scale)
{
    m_scale = scale;
    InvalidateWorldMatrix();
}

// --- Logique du cache ---
void Transform::InvalidateWorldMatrix()
{
    // Si on est déjà dirty, pas besoin de propager à nouveau
    if (!m_isDirty)
    {
        m_isDirty = true;
        // Si ma matrice change, celle de mes enfants aussi
        for (Transform *child : m_children)
        {
            child->InvalidateWorldMatrix();
        }
    }
}

Matrix4x4 Transform::GetLocalMatrix() const
{
    Matrix4x4 transMatrix = Matrix4x4::CreateTranslation(m_position);
    Matrix4x4 rotMatrix = m_orientation.ToRotationMatrix();
    Matrix4x4 scaleMatrix = Matrix4x4::CreateScale(m_scale);
    return transMatrix * rotMatrix * scaleMatrix;
}

// GetWorldMatrix avec cache
const Matrix4x4 &Transform::GetWorldMatrix() const
{
    if (m_isDirty)
    {
        // Si j'ai un parent, ma matrice monde est la sienne multipliée par ma matrice locale.
        if (m_parent)
        {
            m_cachedWorldMatrix = m_parent->GetWorldMatrix() * GetLocalMatrix();
        }
        // si je n'ai pas de parent (objet racine), ma matrice locale est ma matrice monde
        else
        {
            m_cachedWorldMatrix = GetLocalMatrix();
        }
        m_isDirty = false; // Le cache est à jour
    }
    return m_cachedWorldMatrix;
}

Vector3 Transform::GetWorldPosition() const
{
    Matrix4x4 worldMat = GetWorldMatrix();
    return Vector3(worldMat.m[12], worldMat.m[13], worldMat.m[14]);
}

void Transform::SetParent(Transform *newParent)
{
    if (newParent == this)
    {
        return;
    }

    // Refuse toute relation qui créerait un cercle
    for (Transform *cursor = newParent; cursor != nullptr; cursor = cursor->m_parent)
    {
        if (cursor == this)
        {
            return;
        }
    }

    // si j'avais déjà un parent, je m'en détache
    if (m_parent)
    {
        m_parent->RemoveChild(this);
    }

    m_parent = newParent;

    // si un nouveau parent m'a assigné, je m'ajoute à sa liste d'enfants
    if (m_parent)
    {
        m_parent->AddChild(this);
    }
    InvalidateWorldMatrix(); // Changer de parent change la matrice monde
}

void Transform::AddChild(Transform *child)
{
    // On vérifie qu'on ne l'ajoute pas en double
    if (std::find(m_children.begin(), m_children.end(), child) == m_children.end())
    {
        m_children.push_back(child);
    }
}

void Transform::RemoveChild(Transform *child)
{
    // On trouve l'enfant et on le retire du vecteur.
    // std::remove déplace les éléments à supprimer à la fin, puis erase les supprime
    m_children.erase(std::remove(m_children.begin(), m_children.end(), child), m_children.end());
}

// on utilise un namespace anonyme pour que ces fonctions ne soit visibles que depuis ce fichier
namespace
{
    // fonction de création spécifique au Transform
    Component *CreateTransform(GameObject *owner, const nlohmann::json &data)
    {
        Transform *tr = owner->AddComponent<Transform>();
        if (data.contains("position") && data["position"].is_array() && data["position"].size() == 3)
        {
            tr->SetPosition({data["position"][0].get<float>(), data["position"][1].get<float>(), data["position"][2].get<float>()});
        }

        if (data.contains("rotation") && data["rotation"].is_array() && data["rotation"].size() == 3)
        {
            Vector3 eulerAngles = {
                data["rotation"][0].get<float>(),
                data["rotation"][1].get<float>(),
                data["rotation"][2].get<float>()};
            tr->SetOrientation(Quaternion::FromEulerAngles(eulerAngles));
        }

        if (data.contains("scale") && data["scale"].is_array() && data["scale"].size() == 3)
        {
            tr->SetScale({data["scale"][0].get<float>(), data["scale"][1].get<float>(), data["scale"][2].get<float>()});
        }

        return tr;
    }

    // Cette variable statique est initialisée une seule fois au lancement du programme
    // La lambda est immédiatement exécutée, ce qui appelle RegisterComponent.
    const bool isTransformRegistererd = []
    {
        ComponentFactory::Instance().RegisterComponent("Transform", CreateTransform);
        return true;
    }();
}