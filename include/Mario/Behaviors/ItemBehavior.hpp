/**
 * @file ItemBehavior.hpp
 * @brief Behavior for power-up items (Mushroom, Fire Flower, Star, 1UP).
 *        Handles item spawning, bouncing, and power-up effects.
 * @inheritance IEntityBehavior
 */
#ifndef MARIO_ITEM_BEHAVIOR_HPP
#define MARIO_ITEM_BEHAVIOR_HPP

#include "Mario/Behaviors/IEntityBehavior.hpp"

namespace Mario {

/**
 * Implements behavior for collectible power-up items:
 *   - Mushroom: Grants big power (state 1)
 *   - Fire Flower: Grants fire power (state 2)
 *   - Star: Grants invincibility (state 3/4)
 *   - 1UP: Grants extra life
 *
 * Logic ported from C# Entity.cs for power-up spawning and collection.
 */
class ItemBehavior : public IEntityBehavior {
   public:
    enum class ItemType {
        MUSHROOM,     // Big power-up
        FIRE_FLOWER,  // Fire power-up
        STAR,         // Invincibility
        ONE_UP,       // Extra life
        COIN,         // Collectible coin
    };

    explicit ItemBehavior(ItemType type = ItemType::MUSHROOM);
    virtual ~ItemBehavior() = default;

    /**
     * Update item behavior.
     * For spawned items: horizontal movement with bouncing
     * For coins: animation only (no movement)
     * For static items in blocks: no update until collected
     */
    void Update(EntityState& state, const Level& level, const Player& player,
                int gameTimer) override;

    /**
     * Handle player collision - typically collecting the item.
     * Apply the item's effect to the player.
     * @return True if item was collected (should be removed)
     */
    bool OnPlayerCollision(EntityState& state, Player& player,
                           bool isFromAbove) override;

    /**
     * Clone this behavior.
     */
    std::unique_ptr<IEntityBehavior> Clone() const override;

    const char* GetName() const override { return "ItemBehavior"; }

   private:
    ItemType m_Type;
    int m_MoveFrameCounter = 0;  // For movement timing
};

}  // namespace Mario

#endif  // MARIO_ITEM_BEHAVIOR_HPP
