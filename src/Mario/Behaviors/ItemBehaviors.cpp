/**
 * @file ItemBehaviors.cpp
 * @brief Implementations of specialized power-up and collectible item behaviors.
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
#include "Mario/Core/Camera.hpp"
#include "Mario/Level/GameStateManager.hpp"
#include "Mario/UI/UIManager.hpp"
#include "Mario/Services/AudioManager.hpp"
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
    // Physics is applied globally. Mushroom just rolls linearly and drops off ledges.
}

bool MushroomBehavior::OnPlayerCollision([[maybe_unused]] EntityState& state,
                                         [[maybe_unused]] Player& player,
                                         [[maybe_unused]] bool isFromAbove,
                                         [[maybe_unused]] GameStateManager& gameState,
                                         [[maybe_unused]] UIManager& uiManager,
                                         [[maybe_unused]] Camera& camera) {
    return true;
}

std::unique_ptr<IEntityBehavior> MushroomBehavior::Clone() const {
    return std::make_unique<MushroomBehavior>(*this);
}

void MushroomBehavior::OnItemCollected(EntityState& state, Player& player,
                                       GameStateManager& gameState,
                                       UIManager& uiManager, Camera& camera) {
    PlayerState& ps = player.GetState();
    int curState = ps.GetState();

    if (curState == 3) {  // Small Star -> Big Star
        ps.SetY(ps.GetY() - GameConfig::TILE_SIZE);
        ps.SetPowerState(PowerState::BIG_STAR);
        ps.SetMemoryState(PowerState::BIG);
    } else if (curState == 4) {  // Big Star -> no-op
        // Already big
    } else {
        if (curState == 0) {  // Small -> Big
            ps.SetY(ps.GetY() - GameConfig::TILE_SIZE);
            ps.PowerUp(PowerState::BIG);
        }
    }

    AudioManager::GetInstance().PlaySFX(SFXName::Powerup);
    int score = state.GetScoreWorth();
    gameState.AddScore(score);

    float ptsdX = GameConfig::TopLeftToPTSDX(state.GetWorldX(), static_cast<float>(state.GetWidth()), camera.GetOffset());
    float ptsdY = GameConfig::TopLeftToPTSDY(state.GetWorldY(), static_cast<float>(state.GetHeight()));
    uiManager.AddFloatingText(ptsdX, ptsdY, "+" + std::to_string(score), 60);
    state.Delete();
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
                                           [[maybe_unused]] bool isFromAbove,
                                           [[maybe_unused]] GameStateManager& gameState,
                                           [[maybe_unused]] UIManager& uiManager,
                                           [[maybe_unused]] Camera& camera) {
    return true;
}

std::unique_ptr<IEntityBehavior> FireFlowerBehavior::Clone() const {
    return std::make_unique<FireFlowerBehavior>(*this);
}

void FireFlowerBehavior::OnItemCollected(EntityState& state, Player& player,
                                         GameStateManager& gameState,
                                         UIManager& uiManager, Camera& camera) {
    PlayerState& ps = player.GetState();
    int curState = ps.GetState();

    if (curState == 3) {  // Small Star -> Big Star (memory state Fire)
        ps.SetY(ps.GetY() - GameConfig::TILE_SIZE);
        ps.SetPowerState(PowerState::BIG_STAR);
        ps.SetMemoryState(PowerState::FIRE);
    } else if (curState == 4) {  // Big Star -> Big Star (memory state Fire)
        ps.SetMemoryState(PowerState::FIRE);
    } else {
        if (curState == 0) {  // Small -> Fire
            ps.SetY(ps.GetY() - GameConfig::TILE_SIZE);
        }
        ps.PowerUp(PowerState::FIRE);
    }

    AudioManager::GetInstance().PlaySFX(SFXName::Powerup);
    int score = state.GetScoreWorth();
    gameState.AddScore(score);

    float ptsdX = GameConfig::TopLeftToPTSDX(state.GetWorldX(), static_cast<float>(state.GetWidth()), camera.GetOffset());
    float ptsdY = GameConfig::TopLeftToPTSDY(state.GetWorldY(), static_cast<float>(state.GetHeight()));
    uiManager.AddFloatingText(ptsdX, ptsdY, "+" + std::to_string(score), 60);
    state.Delete();
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
                                     [[maybe_unused]] bool isFromAbove,
                                     [[maybe_unused]] GameStateManager& gameState,
                                     [[maybe_unused]] UIManager& uiManager,
                                     [[maybe_unused]] Camera& camera) {
    return true;
}

std::unique_ptr<IEntityBehavior> StarBehavior::Clone() const {
    return std::make_unique<StarBehavior>(*this);
}

void StarBehavior::OnItemCollected(EntityState& state, Player& player,
                                   GameStateManager& gameState,
                                   UIManager& uiManager, Camera& camera) {
    PlayerState& ps = player.GetState();
    ps.StartStar();

    AudioManager::GetInstance().PlaySFX(SFXName::Powerup);
    int score = state.GetScoreWorth();
    gameState.AddScore(score);

    float ptsdX = GameConfig::TopLeftToPTSDX(state.GetWorldX(), static_cast<float>(state.GetWidth()), camera.GetOffset());
    float ptsdY = GameConfig::TopLeftToPTSDY(state.GetWorldY(), static_cast<float>(state.GetHeight()));
    uiManager.AddFloatingText(ptsdX, ptsdY, "+" + std::to_string(score), 60);
    state.Delete();
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
                                      [[maybe_unused]] bool isFromAbove,
                                      [[maybe_unused]] GameStateManager& gameState,
                                      [[maybe_unused]] UIManager& uiManager,
                                      [[maybe_unused]] Camera& camera) {
    return true;
}

std::unique_ptr<IEntityBehavior> OneUpBehavior::Clone() const {
    return std::make_unique<OneUpBehavior>(*this);
}

void OneUpBehavior::OnItemCollected(EntityState& state, Player& player,
                                    GameStateManager& gameState,
                                    UIManager& uiManager, Camera& camera) {
    (void)player;
    gameState.AddLife();
    AudioManager::GetInstance().PlaySFX(SFXName::_1up);

    float ptsdX = GameConfig::TopLeftToPTSDX(state.GetWorldX(), static_cast<float>(state.GetWidth()), camera.GetOffset());
    float ptsdY = GameConfig::TopLeftToPTSDY(state.GetWorldY(), static_cast<float>(state.GetHeight()));
    uiManager.AddFloatingText(ptsdX, ptsdY, "+1UP", 60);

    int score = state.GetScoreWorth();
    gameState.AddScore(score);
    state.Delete();
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
                                     [[maybe_unused]] bool isFromAbove,
                                     [[maybe_unused]] GameStateManager& gameState,
                                     [[maybe_unused]] UIManager& uiManager,
                                     [[maybe_unused]] Camera& camera) {
    return true;
}

std::unique_ptr<IEntityBehavior> CoinBehavior::Clone() const {
    return std::make_unique<CoinBehavior>(*this);
}

float CoinBehavior::GetVisualScaleXModifier(const EntityState& state) const {
    // Procedural coin rotation: 4-frame cycle simulates a spinning coin.
    int frame = state.GetAnimFrame();
    if (frame == 1 || frame == 3) return 0.6f;
    if (frame == 2) return 0.15f;
    return 1.0f;
}

void CoinBehavior::OnItemCollected(EntityState& state, Player& player,
                                   GameStateManager& gameState,
                                   UIManager& uiManager, Camera& camera) {
    (void)player;
    gameState.AddCoin();
    AudioManager::GetInstance().PlaySFX(SFXName::Coin);

    int score = state.GetScoreWorth();
    gameState.AddScore(score);

    float ptsdX = GameConfig::TopLeftToPTSDX(state.GetWorldX(), static_cast<float>(state.GetWidth()), camera.GetOffset());
    float ptsdY = GameConfig::TopLeftToPTSDY(state.GetWorldY(), static_cast<float>(state.GetHeight()));
    uiManager.AddFloatingText(ptsdX, ptsdY, "+" + std::to_string(score), 60);
    state.Delete();
}

}  // namespace Mario
