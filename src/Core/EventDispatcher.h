#pragma once

#include <functional>
#include <map>
#include <vector>
#include <typeindex>
#include <algorithm>
#include "Event.h"

/**
 * @brief Le hub central pour la communication événementielle (Publish-Subscribe).
 * @details permet un découplage complet entre les systèmes
 *
 */
class EventDispatcher
{

public:
    /// Type pour identifier une souscription unique
    using SubscriptionID = unsigned int;

    /// @brief Abonne une fonction à un type d'événement
    /// @tparam T le type d'évenement à écouter
    /// @param callback la fonction à appeler lors de la publication
    /// @return SubscriptionID l'identifiant unique de cette souscription (à conserver pour unsub)
    template <typename T>
    SubscriptionID subscribe(std::function<void(const T &)> callback)
    {
        SubscriptionID id = m_nextID++;
        m_subscribers[std::type_index(typeid(T))].push_back(
            {id, [callback](const Event &event)
             { callback(static_cast<const T &>(event)); }});
        return id;
    }

    /// @brief Publie un événement à tous les abonnés de ce type
    /// @tparam T type d'événement à publier
    /// @param event événement à publier
    template <typename T>
    void publish(const T &event)
    {
        auto it = m_subscribers.find(std::type_index(typeid(T)));
        if (it != m_subscribers.end())
        {
            for (const auto &subscriber : it->second)
            {
                subscriber.callback(event);
            }
        }
    }

    /// @brief Retire une souscription par son id
    /// @param id l'id retourné par subscribe()
    void unsubscribe(SubscriptionID id)
    {
        for (auto &[typeIndex, subscribers] : m_subscribers)
        {
            std::erase_if(subscribers, [id](const Subscriber &s)
                          { return s.id == id; });
        }
    }

private:
    /// Structure interne regroupant ID et son callback
    struct Subscriber
    {
        SubscriptionID id;
        std::function<void(const Event &)> callback;
    };

    SubscriptionID m_nextID = 0;
    std::map<std::type_index, std::vector<Subscriber>> m_subscribers;
};