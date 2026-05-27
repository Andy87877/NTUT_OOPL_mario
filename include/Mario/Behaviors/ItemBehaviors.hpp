/**
 * @file ItemBehaviors.hpp
 * @brief Polymorphic strategies for power-up and collectible items.
 *        Separates Mushrooms, Fire Flowers, Stars, 1UPs, and Coins
 *        into distinct strategy classes following Strategy Pattern.
 * @inheritance IEntityBehavior <- MushroomBehavior
 *              IEntityBehavior <- FireFlowerBehavior
 *              IEntityBehavior <- StarBehavior
 *              IEntityBehavior <- OneUpBehavior
 *              IEntityBehavior <- CoinBehavior
 */
#ifndef MARIO_ITEM_BEHAVIORS_HPP
#define MARIO_ITEM_BEHAVIORS_HPP

#include <memory>

#include "Mario/Behaviors/IEntityBehavior.hpp"

namespace Mario {

// ============================================================================
// MushroomBehavior — standard mushroom power-up (linear movement + gravity)
// ============================================================================
/**
 * Grants Big Mario power when collected. Moves horizontally and falls under gravity.
 * @inheritance IEntityBehavior <- MushroomBehavior
 */
class MushroomBehavior : public IEntityBehavior {
   public:
    MushroomBehavior() = default;
    ~MushroomBehavior() override = default;

    void Update(EntityState& state, const Level& level, const Player& player,
                int gameTimer) override;
    bool OnPlayerCollision(EntityState& state, Player& player,
                           bool isFromAbove) override;
    std::unique_ptr<IEntityBehavior> Clone() const override;
    const char* GetName() const override { return "MushroomBehavior"; }
};

// ============================================================================
// FireFlowerBehavior — fire flower power-up (static item)
// ============================================================================
/**
 * Grants Fire Mario power when collected. Remains static on spawn.
 * @inheritance IEntityBehavior <- FireFlowerBehavior
 */
class FireFlowerBehavior : public IEntityBehavior {
   public:
    FireFlowerBehavior() = default;
    ~FireFlowerBehavior() override = default;

    void Update(EntityState& state, const Level& level, const Player& player,
                int gameTimer) override;
    bool OnPlayerCollision(EntityState& state, Player& player,
                           bool isFromAbove) override;
    std::unique_ptr<IEntityBehavior> Clone() const override;
    const char* GetName() const override { return "FireFlowerBehavior"; }
};

// ============================================================================
// StarBehavior — invincibility star (bounces on ground)
// ============================================================================
/**
 * Grants invincibility star power when collected. Bounces (hops) when grounded.
 * @inheritance IEntityBehavior <- StarBehavior
 */
class StarBehavior : public IEntityBehavior {
   public:
    StarBehavior() = default;
    ~StarBehavior() override = default;

    void Update(EntityState& state, const Level& level, const Player& player,
                int gameTimer) override;
    bool OnPlayerCollision(EntityState& state, Player& player,
                           bool isFromAbove) override;
    std::unique_ptr<IEntityBehavior> Clone() const override;
    const char* GetName() const override { return "StarBehavior"; }
};

// ============================================================================
// OneUpBehavior — green mushroom 1UP (linear movement + gravity)
// ============================================================================
/**
 * Grants an extra life when collected. Moves horizontally and falls under gravity.
 * @inheritance IEntityBehavior <- OneUpBehavior
 */
class OneUpBehavior : public IEntityBehavior {
   public:
    OneUpBehavior() = default;
    ~OneUpBehavior() override = default;

    void Update(EntityState& state, const Level& level, const Player& player,
                int gameTimer) override;
    bool OnPlayerCollision(EntityState& state, Player& player,
                           bool isFromAbove) override;
    std::unique_ptr<IEntityBehavior> Clone() const override;
    const char* GetName() const override { return "OneUpBehavior"; }
};

// ============================================================================
// CoinBehavior — collectible coin (static, animated)
// ============================================================================
/**
 * Awarded score/coin counter increment when collected. Stationary, animated in-place.
 * @inheritance IEntityBehavior <- CoinBehavior
 */
class CoinBehavior : public IEntityBehavior {
   public:
    CoinBehavior() = default;
    ~CoinBehavior() override = default;

    void Update(EntityState& state, const Level& level, const Player& player,
                int gameTimer) override;
    bool OnPlayerCollision(EntityState& state, Player& player,
                           bool isFromAbove) override;
    std::unique_ptr<IEntityBehavior> Clone() const override;
    const char* GetName() const override { return "CoinBehavior"; }
};

}  // namespace Mario

#endif  // MARIO_ITEM_BEHAVIORS_HPP
