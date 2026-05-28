/**
 * @file ItemBehaviors.cpp
 * @brief Implementations of specialized power-up and collectible item
 * behaviors.
 * @inheritance IEntityBehavior <- MushroomBehavior
 *              IEntityBehavior <- FireFlowerBehavior
 *              IEntityBehavior <- StarBehavior
 *              IEntityBehavior <- OneUpBehavior
 *              IEntityBehavior <- CoinBehavior
 */
#include "Mario/Behaviors/ItemBehaviors.hpp"

#include "Mario/Core/Collider.hpp"
#include "Mario/Core/PhysicsEngine.hpp"
#include "Mario/Level/EntityState.hpp"
#include "Mario/Level/Level.hpp"
#include "Mario/Player/Player.hpp"
#include "Mario/Player/PlayerState.hpp"
#include "Util/Logger.hpp"

namespace Mario {

// ============================================================================
// MushroomBehavior
// ============================================================================

void MushroomBehavior::Update(EntityState& state,
                              [[maybe_unused]] const Level& level,
                              [[maybe_unused]] const Player& player,
                              [[maybe_unused]] int gameTimer) {
    if (state.IsStatic()) return;
    // Physics is applied globally. Mushroom just rolls linearly and drops off
    // ledges.
}

bool MushroomBehavior::OnPlayerCollision([[maybe_unused]] EntityState& state,
                                         [[maybe_unused]] Player& player,
                                         [[maybe_unused]] bool isFromAbove) {
    // Mushroom collected (power-up resolution is driven by PlayerEntityHandler)
    return true;
}

std::unique_ptr<IEntityBehavior> MushroomBehavior::Clone() const {
    return std::make_unique<MushroomBehavior>(*this);
}

// ============================================================================
// FireFlowerBehavior
// ============================================================================

void FireFlowerBehavior::Update([[maybe_unused]] EntityState& state,
                                [[maybe_unused]] const Level& level,
                                [[maybe_unused]] const Player& player,
                                [[maybe_unused]] int gameTimer) {
    // Fire Flower remains completely static.
}

bool FireFlowerBehavior::OnPlayerCollision([[maybe_unused]] EntityState& state,
                                           [[maybe_unused]] Player& player,
                                           [[maybe_unused]] bool isFromAbove) {
    // Fire Flower collected
    return true;
}

std::unique_ptr<IEntityBehavior> FireFlowerBehavior::Clone() const {
    return std::make_unique<FireFlowerBehavior>(*this);
}

// ============================================================================
// StarBehavior
// ============================================================================

void StarBehavior::Update(EntityState& state,
                          [[maybe_unused]] const Level& level,
                          [[maybe_unused]] const Player& player,
                          [[maybe_unused]] int gameTimer) {
    if (state.IsStatic()) return;

    // Star hops up whenever it makes contact with the ground
    if (state.IsGrounded()) {
        state.SetGrounded(false);
        state.SetFallHeight(20.0);  // Classic star bounce height
    }
}

bool StarBehavior::OnPlayerCollision([[maybe_unused]] EntityState& state,
                                     [[maybe_unused]] Player& player,
                                     [[maybe_unused]] bool isFromAbove) {
    // Star collected
    return true;
}

std::unique_ptr<IEntityBehavior> StarBehavior::Clone() const {
    return std::make_unique<StarBehavior>(*this);
}

// ============================================================================
// OneUpBehavior
// ============================================================================

void OneUpBehavior::Update(EntityState& state,
                           [[maybe_unused]] const Level& level,
                           [[maybe_unused]] const Player& player,
                           [[maybe_unused]] int gameTimer) {
    if (state.IsStatic()) return;
    // Green mushroom 1UP rolls linearly and falls under gravity.
}

bool OneUpBehavior::OnPlayerCollision([[maybe_unused]] EntityState& state,
                                      [[maybe_unused]] Player& player,
                                      [[maybe_unused]] bool isFromAbove) {
    // 1UP collected
    return true;
}

std::unique_ptr<IEntityBehavior> OneUpBehavior::Clone() const {
    return std::make_unique<OneUpBehavior>(*this);
}

// ============================================================================
// CoinBehavior
// ============================================================================

void CoinBehavior::Update([[maybe_unused]] EntityState& state,
                          [[maybe_unused]] const Level& level,
                          [[maybe_unused]] const Player& player,
                          [[maybe_unused]] int gameTimer) {
    // Coins do not move, only animate in-place.
}

bool CoinBehavior::OnPlayerCollision([[maybe_unused]] EntityState& state,
                                     [[maybe_unused]] Player& player,
                                     [[maybe_unused]] bool isFromAbove) {
    // Coin collected
    return true;
}

std::unique_ptr<IEntityBehavior> CoinBehavior::Clone() const {
    return std::make_unique<CoinBehavior>(*this);
}

float CoinBehavior::GetVisualScaleXModifier(const EntityState& state) const {
    // Procedural coin rotation: 4-frame cycle simulates a spinning coin.
    // frame 0: full width (1.0)
    // frame 1: medium width (0.6)
    // frame 2: thin line width (0.15)
    // frame 3: medium width (0.6)
    int frame = state.GetAnimFrame();
    if (frame == 1 || frame == 3) return 0.6f;
    if (frame == 2) return 0.15f;
    return 1.0f;
}

}  // namespace Mario
