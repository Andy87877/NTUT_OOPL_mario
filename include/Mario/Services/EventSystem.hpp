/**
 * @file EventSystem.hpp
 * @brief Simple event publish-subscribe system for game events.
 *        Allows loose coupling between game components via events.
 * @inheritance None (Service)
 */
#ifndef MARIO_EVENT_SYSTEM_HPP
#define MARIO_EVENT_SYSTEM_HPP

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace Mario {

/**
 * Generic event listener callback type.
 */
template <typename EventType>
using EventListener = std::function<void(const EventType&)>;

/**
 * Simple event system for decoupled communication.
 * Example usage:
 *   EventSystem<PlayerDeadEvent> events;
 *   events.Subscribe(listenerFunc);
 *   events.Publish(PlayerDeadEvent{...});
 */
template <typename EventType>
class EventSystem {
   public:
    /**
     * Subscribe to events of this type.
     * @return Subscription ID (can be used to unsubscribe)
     */
    int Subscribe(EventListener<EventType> listener) {
        int id = m_NextId++;
        m_Listeners[id] = listener;
        return id;
    }

    /**
     * Unsubscribe from events.
     */
    void Unsubscribe(int subscriptionId) { m_Listeners.erase(subscriptionId); }

    /**
     * Publish an event to all subscribers.
     */
    void Publish(const EventType& event) {
        for (auto& [id, listener] : m_Listeners) {
            listener(event);
        }
    }

    /**
     * Clear all subscribers.
     */
    void Clear() { m_Listeners.clear(); }

   private:
    int m_NextId = 0;
    std::map<int, EventListener<EventType>> m_Listeners;
};

}  // namespace Mario

#endif  // MARIO_EVENT_SYSTEM_HPP
