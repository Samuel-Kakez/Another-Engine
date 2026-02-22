#pragma once

#include <functional>
#include <map>
#include <string>
#include <nlohmann/json.hpp>

// Déclarations anticipées
class Component;
class GameObject;

// Alias pour notre type de fonction de création pour plus de lisibilité.
// La fonction prend le GameObject propriétaire et les données JSON, et retourne un Component.
using ComponentCreator = std::function<Component *(GameObject *, const nlohmann::json &)>;

/**
 * @brief Usine pour créer des composants à partir d'un nom de type.
 * @details utilise le patron de conception Singleton pour un accès global et centralisé.
 * Les composants s'enregistrent eux-mêmes auprès de cette using au démarrage du programme,
 * ce qui permet d'ajouter de nouveaux composants sans modifier le code existant.
 */
class ComponentFactory
{
public:
    /**
     * @brief Accède à l'instance unique (Singleton) de la Factory.
     * @details Implémente le "Meyers' Singleton", qui est un thread-safe en C++11+
     *
     * @return ComponentFactory& Une référence vers l'instance unique.
     */
    static ComponentFactory &Instance();

    /**
     * @brief Enregistre un nouveau type de composant dans la factory.
     * @details Cette méthode est appelée une seule fois par type de composant au démarrage
     * du programme via un mécanisme d'auto-enregistrement.
     *
     * @param typeName Le nom du composant (chaîne utilisée dans le JSON)
     * @param creator La fonction de callback capable de créer une instance de ce composant
     */
    void RegisterComponent(const std::string &typeName, ComponentCreator creator);

    /**
     * @brief Créé une instance d'un composant en utilisant son nom de type.
     * @details Cherche le nom de type dans son registre et, s'il est trouvé, appelle
     * la fonction de création correspondante.
     *
     * @param typeName le nom du composant à créer (doit correspondre à un type enregistré)
     * @param owner le GameObject auquel le nouveau composant sera attaché
     * @param data les données JSON contenant les propriétés spécifiques pour ce composant.
     * @return Component* un pointeur vers le composant créé, ou nullptr si le type est inconnu.
     */
    Component *CreateComponent(const std::string &typeName, GameObject *owner, const nlohmann::json &data);

private:
    // Le patron Singleton nécessite des constructeurs/opérateurs privés pour empêcher la création d'instances multiples.
    ComponentFactory() = default;
    ~ComponentFactory() = default;

    ComponentFactory(const ComponentFactory &) = delete;
    ComponentFactory &operator=(const ComponentFactory &) = delete;

    /// @brief Map qui stocke les fonctions de création associées à leur nom de type
    std::map<std::string, ComponentCreator> m_creators;
};
