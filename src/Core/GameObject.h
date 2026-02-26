#pragma once

#include <vector>
#include <map>
#include <typeindex> // Inclus pour std::type_index
#include <memory>
#include <utility>
#include <string>

#include "Core/Component.h"
#include "Core/Scene.h"
#include "Core/Event.h"
#include "Core/EventDispatcher.h"

/**
 * @brief L'entité de base dans la scène
 * @details Un GameObject est un conteneur pour des composants. Il ne possède aucune donnée ou
 * comportement propre, mais les acquiert via les composants qui lui sont attachés
 * Il possède désormais un nom pour l'identification
 */
class GameObject
{
public:
    /**
     * @brief Constructeur par défaut de GameObject
     * @param name Le nom de l'objet, GameObject par défaut
     */
    GameObject(const std::string &name = "GameObject");

    /**
     * @brief Destructeur. Libère la mémoire de tous les composants attachés
     *
     */
    ~GameObject();

    /**
     * @brief Appelle la méthode Update() de tous les composants attachés.
     *
     * @param deltaTime Le temps écoulé depuis la dernière frame en secondes
     */
    void Update(float deltaTime);

    template <typename T, typename... Args>
    T *AddComponent(Args &&...args);

    /**
     * @brief Récupère un composant par son type, avec support du polymorphisme
     *
     * @tparam T Le type du composant à récupérer (Transform, Light...)
     * @return T* Un pointeur vers le composant du type T, ou nullptr s'il n'est pas trouvé
     */
    template <typename T>
    T *GetComponent()
    {
        // on utilise maintenant std::type_index pour une recherche sûre et rapide
        // D'abord, on tente une recherche rapide dans la map pour les types exacts
        auto it = m_componentMap.find(std::type_index(typeid(T)));
        if (it != m_componentMap.end())
        {
            // on utilise static_cast car on est sûr du type grâce à notre recherche
            return static_cast<T *>(it->second);
        }
        // si non trouvé (cas recherche par classe de base), on parcourt tous les composants
        for (const auto &component : m_components)
        {
            // On teste dynamic_cast qui gère le polymorphisme en toute sécurité
            T *result = dynamic_cast<T *>(component.get());
            if (result != nullptr)
            {
                // On a trouvé un composant qui hérite de T
                return result;
            }
        }

        return nullptr;
    }

    /// @brief Récupère tous les composants d'un type donné, avec support du polymorphisme
    /// @tparam T le type du composant à chercher
    /// @return un vecteur de pointeurs vers tous les composants du type T
    template <typename T>
    std::vector<T *> GetComponents()
    {
        std::vector<T *> results;
        for (const auto &component : m_components)
        {
            T *result = dynamic_cast<T *>(component.get());
            if (result != nullptr)
            {
                results.push_back(result);
            }
        }
        return results;
    }

    /**
     * @brief Marque cet objet pour qu'il soit détruit à la fin de la frame
     *
     */
    void Destroy() { m_lifecycleState = LifecycleState::PendingDestroy; }

    /**
     * @brief Vérifie si l'objet est en attente de destruction
     */
    bool IsPendingKill() const { return m_lifecycleState == LifecycleState::PendingDestroy; }

    bool IsInitialized() const { return m_lifecycleState == LifecycleState::Initialized; }
    void MarkInitialized()
    {
        if (m_lifecycleState == LifecycleState::Constructing)
        {
            m_lifecycleState = LifecycleState::Initialized;
        }
    }

    /**
     * @brief Le nom du GameObject, pour identification
     *
     */
    std::string name;

    Scene &GetScene() const { return *m_ownerScene; }

    /**
     * @brief Appelle la méthode FixedUpdate() de tous les composants attachés
     *
     * @param fixedDeltaTime Le pas de temps fixe
     */
    void FixedUpdate(float fixedDeltaTime);

private:
    friend class Scene;
    /**
     * @brief Pointeur non-propriétaire vers la scène qui contient de GameObject.
     * @details Permet à l'objet et à ses composants de communiquer avec les systèmes de la scène
     * ex : pour s'enregistrer auprès d'un manager
     *
     */
    Scene *m_ownerScene = nullptr;
    // une liste pour un parcours rapide (pour Update)
    std::vector<std::unique_ptr<Component>> m_components;
    // une carte pour une recherche rapide par type, en utilisant type_index comme clé
    std::map<std::type_index, Component *> m_componentMap;

    enum class LifecycleState
    {
        Constructing,
        Initialized,
        PendingDestroy
    };

    LifecycleState m_lifecycleState = LifecycleState::Constructing;
};

/**
 * @brief Le composant est créé et sa propriété est transférée au GameObject.
 * @details Après l'ajout, un événement ComponentAddedEvent est publié
 *
 * @tparam T
 * @tparam Args
 * @param args
 * @return T*
 */
template <typename T, typename... Args>
T *GameObject::AddComponent(Args &&...args)
{
    // on créé le composant en tant que pointeur unique, en transférant les arguments
    auto newComponent = std::make_unique<T>(std::forward<Args>(args)...);

    // On récupère un pointeur brut pour le retourner et pour la map
    T *ptr = newComponent.get();
    ptr->gameObject = this;

    m_componentMap[std::type_index(typeid(T))] = ptr;
    m_components.push_back(std::move(newComponent));

    if (m_ownerScene)
    {
        m_ownerScene->GetEventDispatcher().publish(ComponentAddedEvent(ptr));
    }

    return ptr;
}
