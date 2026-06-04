/**
 * @file BehaviorRegistry.hpp
 * @brief Dynamic registry for mapping EntityType to behavior strategy creators (OCP).
 * @inheritance None
 */
#ifndef MARIO_BEHAVIOR_REGISTRY_HPP
#define MARIO_BEHAVIOR_REGISTRY_HPP

#include <memory>
#include <functional>
#include <unordered_map>
#include "Mario/Level/EntityDef.hpp"
#include "Mario/Behaviors/IEntityBehavior.hpp"

namespace Mario {

class BehaviorRegistry {
   public:
    using Creator = std::function<std::unique_ptr<IEntityBehavior>(const EntityDef& def)>;

    /**
     * Resolve and construct a behavior strategy polymorphically based on type.
     */
    static std::unique_ptr<IEntityBehavior> Create(EntityType type, const EntityDef& def);

   private:
    static void InitializeRegistry(std::unordered_map<EntityType, Creator>& map);
};

} // namespace Mario

#endif // MARIO_BEHAVIOR_REGISTRY_HPP
