/**
 * @file ServiceLocator.hpp
 * @brief Service locator for dependency injection and service registration.
 *        Provides centralized access to shared services throughout the game.
 * @inheritance None (Service locator pattern)
 */
#ifndef MARIO_SERVICE_LOCATOR_HPP
#define MARIO_SERVICE_LOCATOR_HPP

#include <map>
#include <memory>
#include <typeinfo>

#include "Mario/Services/IAudioService.hpp"

namespace Mario {

/**
 * Centralized service locator for game services.
 * Provides single point of access for shared services like audio, events, etc.
 * Follows Service Locator pattern for dependency availability.
 */
class ServiceLocator {
   public:
    /**
     * Get the singleton instance.
     */
    static ServiceLocator& GetInstance() {
        static ServiceLocator instance;
        return instance;
    }

    /**
     * Register a service by type.
     */
    template <typename ServiceType>
    void RegisterService(std::shared_ptr<ServiceType> service) {
        const auto& key = typeid(ServiceType).name();
        m_Services[key] = service;
    }

    /**
     * Get a service by type.
     */
    template <typename ServiceType>
    std::shared_ptr<ServiceType> GetService() {
        const auto& key = typeid(ServiceType).name();
        auto it = m_Services.find(key);
        if (it != m_Services.end()) {
            return std::static_pointer_cast<ServiceType>(it->second);
        }
        return nullptr;
    }

    /**
     * Check if a service is registered.
     */
    template <typename ServiceType>
    bool HasService() const {
        return m_Services.find(typeid(ServiceType).name()) != m_Services.end();
    }

    /**
     * Clear all services.
     */
    void Clear() { m_Services.clear(); }

   private:
    ServiceLocator() = default;
    ~ServiceLocator() = default;

    std::map<std::string, std::shared_ptr<void>> m_Services;
};

}  // namespace Mario

#endif  // MARIO_SERVICE_LOCATOR_HPP
