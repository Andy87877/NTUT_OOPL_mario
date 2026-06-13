/**
 * @file ItemBehaviors.hpp
 * @brief Polymorphic strategies for power-up and collectible items.
 *        Separates Mushrooms, Fire Flowers, Stars, 1UPs, and Coins
 *        into distinct strategy classes following Strategy Pattern.
 * @inheritance IEntityBehavior -> ItemBehavior -> MushroomBehavior
 *              IEntityBehavior -> ItemBehavior -> FireFlowerBehavior
 *              IEntityBehavior -> ItemBehavior -> StarBehavior
 *              IEntityBehavior -> ItemBehavior -> OneUpBehavior
 *              IEntityBehavior -> ItemBehavior -> CoinBehavior
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
 * Grants Big Mario power when collected. Moves horizontally and falls under
 * gravity.
 * @inheritance IEntityBehavior <- MushroomBehavior
 */
class MushroomBehavior : public ItemBehavior {
   public:
    MushroomBehavior() = default;
    ~MushroomBehavior() override = default;

    void Update(EntityState& state, const Level& level, const Player& player,
                int gameTimer) override;
    bool OnPlayerCollision(EntityState& state, Player& player,
                           bool isFromAbove, GameStateManager& gameState,
                           UIManager& uiManager, Camera& camera) override;
    std::unique_ptr<IEntityBehavior> Clone() const override;
    const char* GetName() const override { return "MushroomBehavior"; }
    void OnItemCollected(EntityState& state, Player& player,
                         GameStateManager& gameState,
                         UIManager& uiManager, Camera& camera) override;
};

// ============================================================================
// FireFlowerBehavior — fire flower power-up (static item)
// ============================================================================
/**
 * Grants Fire Mario power when collected. Remains static on spawn.
 * @inheritance IEntityBehavior <- FireFlowerBehavior
 */
class FireFlowerBehavior : public ItemBehavior {
   public:
    FireFlowerBehavior() = default;
    ~FireFlowerBehavior() override = default;

    void Update(EntityState& state, const Level& level, const Player& player,
                int gameTimer) override;
    bool OnPlayerCollision(EntityState& state, Player& player,
                           bool isFromAbove, GameStateManager& gameState,
                           UIManager& uiManager, Camera& camera) override;
    std::unique_ptr<IEntityBehavior> Clone() const override;
    const char* GetName() const override { return "FireFlowerBehavior"; }
    void OnItemCollected(EntityState& state, Player& player,
                         GameStateManager& gameState,
                         UIManager& uiManager, Camera& camera) override;
};

// ============================================================================
// StarBehavior — invincibility star (bounces on ground)
// ============================================================================
/**
 * Grants invincibility star power when collected. Bounces (hops) when grounded.
 * @inheritance IEntityBehavior <- StarBehavior
 */
class StarBehavior : public ItemBehavior {
   public:
    StarBehavior() = default;
    ~StarBehavior() override = default;

    void Update(EntityState& state, const Level& level, const Player& player,
                int gameTimer) override;
    bool OnPlayerCollision(EntityState& state, Player& player,
                           bool isFromAbove, GameStateManager& gameState,
                           UIManager& uiManager, Camera& camera) override;
    std::unique_ptr<IEntityBehavior> Clone() const override;
    const char* GetName() const override { return "StarBehavior"; }
    void OnItemCollected(EntityState& state, Player& player,
                         GameStateManager& gameState,
                         UIManager& uiManager, Camera& camera) override;
};

// ============================================================================
// OneUpBehavior — green mushroom 1UP (linear movement + gravity)
// ============================================================================
/**
 * Grants an extra life when collected. Moves horizontally and falls under
 * gravity.
 * @inheritance IEntityBehavior <- OneUpBehavior
 */
class OneUpBehavior : public ItemBehavior {
   public:
    OneUpBehavior() = default;
    ~OneUpBehavior() override = default;

    void Update(EntityState& state, const Level& level, const Player& player,
                int gameTimer) override;
    bool OnPlayerCollision(EntityState& state, Player& player,
                           bool isFromAbove, GameStateManager& gameState,
                           UIManager& uiManager, Camera& camera) override;
    std::unique_ptr<IEntityBehavior> Clone() const override;
    const char* GetName() const override { return "OneUpBehavior"; }
    void OnItemCollected(EntityState& state, Player& player,
                         GameStateManager& gameState,
                         UIManager& uiManager, Camera& camera) override;
};

// ============================================================================
// CoinBehavior — collectible coin (static, animated)
// ============================================================================
/**
 * Awarded score/coin counter increment when collected. Stationary, animated
 * in-place.
 * @inheritance IEntityBehavior <- CoinBehavior
 */
class CoinBehavior : public ItemBehavior {
   public:
    CoinBehavior() = default;
    ~CoinBehavior() override = default;

    void Update(EntityState& state, const Level& level, const Player& player,
                int gameTimer) override;
    bool OnPlayerCollision(EntityState& state, Player& player,
                           bool isFromAbove, GameStateManager& gameState,
                           UIManager& uiManager, Camera& camera) override;
    std::unique_ptr<IEntityBehavior> Clone() const override;
    const char* GetName() const override { return "CoinBehavior"; }
    float GetVisualScaleXModifier(const EntityState& state) const override;
    void OnItemCollected(EntityState& state, Player& player,
                         GameStateManager& gameState,
                         UIManager& uiManager, Camera& camera) override;
};

}  // namespace Mario

#endif  // MARIO_ITEM_BEHAVIORS_HPP
