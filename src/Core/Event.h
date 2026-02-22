#pragma once

class Component;
class GameObject;

/**
 * @brief Structure de base pour tous les types d'événements.
 * @details Ajout d'un destructeur virtuel pour rendre la classe polymorphique
 */
struct Event
{
    /**
     * @brief Destructeur virtuel par défaut
     * @details La présence d'au moins une fonction virtuelle
     * rend la classe polymorphique, ce qui est une condition requite pour que
     * dynamic_cast puisse fonctionner.
     *
     */
    virtual ~Event() = default;
};

/**
 * @brief Événement publié lors de l'ajout d'un composant.
 *
 */
struct ComponentAddedEvent : public Event
{
    Component *component;
    /**
     * @brief Constructeur explicite pour initialiser l'événement
     *
     * @param comp le composant qui a été ajouté
     */
    explicit ComponentAddedEvent(Component *comp) : component(comp) {}
};

/**
 * @brief Événement publié avant la destruction d'un GameObject.
 *
 */
struct GameObjectWillBeDestroyedEvent : public Event
{
    GameObject *gameObject;

    /**
     * @brief Constructeur explicite pour initialiser l'événement
     *
     * @param go le GameObject qui va être détruit
     */
    explicit GameObjectWillBeDestroyedEvent(GameObject *go) : gameObject(go) {}
};

/**
 * @brief Événement publié lorsque la scène est entièrement vidée via Scene::Clear()
 * @details ne contient pas de données ; son type seul sert de message pour que les systèmes abonnées puissent réinitialiser leur état.
 */
struct SceneClearedEvent : public Event
{
};

/**
 * @brief Evenement publié après qu'un GameObject ait été entièrement initialisé depuis le JSON
 * @details c'est le signal pour les systèmes qu'ils peuvent intéragir avec l'objet en toute sécurité, une fois la configuration 
 * de tous ses composants.
 * 
 */
struct GameObjectInitializedEvent : public Event{
    GameObject* gameObject;
    explicit GameObjectInitializedEvent(GameObject* go) : gameObject(go) {}
};