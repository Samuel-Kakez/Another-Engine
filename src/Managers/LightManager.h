#pragma once

#include <vector>
#include "Renderer/RenderSettings.h"
#include "Core/EventDispatcher.h"

// déclarations anticipées
// nouveau : déclarations pour les événements et le dispatcher
class DirectionalLight;
struct ComponentAddedEvent;
struct GameObjectWillBeDestroyedEvent;
struct GameObjectInitializedEvent;
struct SceneClearedEvent;

/**
 * @brief Gère le registre de toutes les lumières et leurs ressources associées (ex: shadow cubemaps)
 * @details Centralise l'accès aux lumières de manière optimisée, et gère le cycle de vie de leurs ressources
 *
 */
class LightManager
{
public:
    /**
     * @brief Le constructeur prend maintenant le dispatcher en dépendance
     * @details S'abonne aux événements pour gérer automatiquement le registre des lumières
     * @param dispatcher Le dispatcher d'événements du moteur.
     */
    LightManager(EventDispatcher &dispatcher);

    /// @brief Destructeur : s'assure de détruire les ressources
    ~LightManager();

    /// @brief Retourne la lumière directionnelle active (peut être nulle)
    /// @return Pointeur vers la lumière directionnelle
    DirectionalLight* GetDirectionalLight() const;

    /**
     * @brief Vide le registre de toutes les lumières
     * @details doit être appelé lors du déchargement d'une scène ou à la fin du programme
     */
    void Clear();

private:
    /**
     * @brief Enregistre une lumière
     *
     * @param light un pointeur vers le composant Light à ajouter
     */
    void RegisterLight(DirectionalLight *light);

    /**
     * @brief Désenregistre une lumière (lorsqu'elle est détruite)
     *
     * @param light un pointeur vers le composant Light à retirer
     */
    void UnregisterLight(DirectionalLight *light);

    // --- Gestion des ressources d'ombres ---

    /**
     * @brief Crée la cubemap de profondeur pour une lumière données
     *
     * @param light
     * @param settings
     */
    void CreateShadowResourcesFor(DirectionalLight *light);

    /**
     * @brief Détruit la cubemap de profondeur pour une lumière donnée
     *
     * @param light
     */
    void DestroyShadowResourcesFor(DirectionalLight *light);

    // Callbacks pour réagir aux événements.
    void OnComponentAdded(const ComponentAddedEvent &event);
    void OnGameObjectDestroyed(const GameObjectWillBeDestroyedEvent &event);
    void OnGameObjectInitialized(const GameObjectInitializedEvent& event);

    // Stockage pour le désabonnement 
    EventDispatcher* m_dispatcher = nullptr;
    std::vector<EventDispatcher::SubscriptionID> m_subscriptionIDs;

    // Le conteneur de stockage.
    DirectionalLight* m_DirectionalLight = nullptr;
};