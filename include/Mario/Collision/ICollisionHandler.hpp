/**
 * @file ICollisionHandler.hpp
 * @brief Abstract base interface for all collision handler strategies.
 *        Defines a common base for the four collision subsystems so the design
 *        is visible in the class hierarchy: Player-Block, Player-Entity,
 *        Entity-Block, and Entity-Entity handlers all inherit this.
 * @inheritance None (pure abstract interface)
 */
#ifndef MARIO_COLLISION_I_COLLISION_HANDLER_HPP
#define MARIO_COLLISION_I_COLLISION_HANDLER_HPP

namespace Mario {

/**
 * Marker interface for all collision handler strategies.
 * Concrete handlers inherit from ICollisionHandler to express their role
 * in the per-frame collision pipeline and to allow future polymorphic
 * extension without modifying CollisionManager.
 */
class ICollisionHandler {
   public:
    virtual ~ICollisionHandler() = default;
};

}  // namespace Mario

#endif  // MARIO_COLLISION_I_COLLISION_HANDLER_HPP
