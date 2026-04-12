/**
 * @file BowserBehavior.cpp
 * @brief Implementation of Bowser boss AI behavior for 8-4.
 * @inheritance IEntityBehavior <- BowserBehavior
 */
#include "Mario/Behaviors/BowserBehavior.hpp"

#include "Mario/AudioManager.hpp"
#include "Mario/Collider.hpp"
#include "Mario/EntityFactory.hpp"
#include "Mario/EntityState.hpp"
#include "Mario/Level.hpp"
#include "Mario/PhysicsEngine.hpp"
#include "Mario/Player.hpp"
#include "Mario/PlayerState.hpp"
#include "Util/Logger.hpp"

namespace Mario {

BowserBehavior::BowserBehavior()
    : m_Phase(BowserPhase::PATROL),
      m_PhaseTimer(0),
      m_AttackCounter(0),
      m_HealthPoints(3),
      m_DamageFlashCounter(0),
      m_PatrolDirection(1) {}

void BowserBehavior::Update(EntityState& state, const Level& level,
                            const Player& player, int gameTimer) {
    if (m_Phase == BowserPhase::DEFEATED) {
        UpdateDefeated(state);
        return;
    }

    if (m_Phase == BowserPhase::DAMAGED) {
        UpdateDamaged(state);
        return;
    }

    m_PhaseTimer++;

    switch (m_Phase) {
        case BowserPhase::PATROL:
            UpdatePatrol(state, level, player);
            break;
        case BowserPhase::FIRE_ATTACK:
            // Fire attack handled separately - no entity factory call for now
            UpdateFireAttackPhase();
            break;
        case BowserPhase::JUMP_ATTACK:
            UpdateJumpAttack(state, level, player);
            break;
        default:
            break;
    }

    // Phase transition logic
    if (m_PhaseTimer >= PATROL_PHASE_LENGTH && m_Phase == BowserPhase::PATROL) {
        m_Phase = BowserPhase::FIRE_ATTACK;
        m_PhaseTimer = 0;
        m_AttackCounter = 0;
    } else if (m_PhaseTimer >= FIRE_ATTACK_LENGTH &&
               m_Phase == BowserPhase::FIRE_ATTACK) {
        m_Phase = BowserPhase::JUMP_ATTACK;
        m_PhaseTimer = 0;
    } else if (m_PhaseTimer >= 240 && m_Phase == BowserPhase::JUMP_ATTACK) {
        m_Phase = BowserPhase::PATROL;
        m_PhaseTimer = 0;
    }
}

void BowserBehavior::UpdatePatrol(EntityState& state, const Level& level,
                                  const Player& player) {
    // Apply gravity
    double fallHeight = state.GetFallHeight();
    double velY = state.GetVelY();
    float yDelta =
        PhysicsEngine::ApplyGravity(fallHeight, velY, state.IsGrounded());
    state.SetFallHeight(fallHeight);
    state.SetVelY(velY);
    state.SetWorldY(state.GetWorldY() + yDelta);

    float moveSpeed = 3.0f;  // Slower than regular enemies
    float xDelta = moveSpeed * m_PatrolDirection;
    state.SetWorldX(state.GetWorldX() + xDelta);

    // Check for walls to reverse patrol direction (using grid-based collision
    // like Mario)
    AABB bowserBox = state.GetCollider();
    int topTile = static_cast<int>(bowserBox.top) / GameConfig::TILE_SIZE;
    int bottomTile =
        static_cast<int>(bowserBox.bottom - 1) / GameConfig::TILE_SIZE;

    bool hitWall = false;

    // Check collision in the direction of movement
    if (m_PatrolDirection > 0) {
        // Moving right: check right side of Bowser
        int rightTile =
            static_cast<int>(bowserBox.right) / GameConfig::TILE_SIZE;
        for (int y = topTile; y <= bottomTile; y++) {
            const Block* block = level.GetBlockAt(rightTile, y);
            if (block && block->IsSolid()) {
                AABB blockBox = block->GetAABB();
                if (bowserBox.Intersects(blockBox)) {
                    hitWall = true;
                    break;
                }
            }
        }
    } else {
        // Moving left: check left side of Bowser
        int leftTile = static_cast<int>(bowserBox.left) / GameConfig::TILE_SIZE;
        for (int y = topTile; y <= bottomTile; y++) {
            const Block* block = level.GetBlockAt(leftTile, y);
            if (block && block->IsSolid()) {
                AABB blockBox = block->GetAABB();
                if (bowserBox.Intersects(blockBox)) {
                    hitWall = true;
                    break;
                }
            }
        }
    }

    if (hitWall) {
        m_PatrolDirection *= -1;  // Reverse direction
    }

    // Check ground
    AABB footprint = state.GetCollider();
    footprint.top = footprint.bottom;
    footprint.bottom += GameConfig::GRAVITY_ACCELERATION;

    bool isGrounded = false;
    for (const auto& block : level.GetAllBlocks()) {
        if (block && footprint.Intersects(block->GetAABB())) {
            isGrounded = true;
            break;
        }
    }

    state.SetGrounded(isGrounded);
    state.SetDirection(m_PatrolDirection > 0 ? 1 : 0);
}

void BowserBehavior::UpdateFireAttackPhase() {
    m_AttackCounter++;
    if (m_AttackCounter == 10) {
        Mario::AudioManager::GetInstance().PlaySFX(Mario::SFXName::EnemyFire);
    }
    // Could spawn fireballs periodically during this phase
    // Actual fireball spawning would be called from App level
}

void BowserBehavior::UpdateJumpAttack(EntityState& state, const Level& level,
                                      const Player& player) {
    // Similar to patrol but with occasional jumps
    double fallHeight = state.GetFallHeight();
    double velY = state.GetVelY();
    float yDelta =
        PhysicsEngine::ApplyGravity(fallHeight, velY, state.IsGrounded());
    state.SetFallHeight(fallHeight);
    state.SetVelY(velY);
    state.SetWorldY(state.GetWorldY() + yDelta);

    float moveSpeed = 5.0f;  // Faster during jump phase
    float xDelta = moveSpeed * m_PatrolDirection;
    state.SetWorldX(state.GetWorldX() + xDelta);

    // Jump randomly toward player
    if (m_PhaseTimer % 80 == 0 && state.IsGrounded()) {
        state.SetFallHeight(50.0f);  // High jump
    }

    // Check walls and ground (using grid-based collision like Mario)
    AABB bowserBox = state.GetCollider();
    int topTile = static_cast<int>(bowserBox.top) / GameConfig::TILE_SIZE;
    int bottomTile =
        static_cast<int>(bowserBox.bottom - 1) / GameConfig::TILE_SIZE;

    bool hitWall = false;

    // Check collision in the direction of movement
    if (m_PatrolDirection > 0) {
        // Moving right: check right side of Bowser
        int rightTile =
            static_cast<int>(bowserBox.right) / GameConfig::TILE_SIZE;
        for (int y = topTile; y <= bottomTile; y++) {
            const Block* block = level.GetBlockAt(rightTile, y);
            if (block && block->IsSolid()) {
                AABB blockBox = block->GetAABB();
                if (bowserBox.Intersects(blockBox)) {
                    hitWall = true;
                    break;
                }
            }
        }
    } else {
        // Moving left: check left side of Bowser
        int leftTile = static_cast<int>(bowserBox.left) / GameConfig::TILE_SIZE;
        for (int y = topTile; y <= bottomTile; y++) {
            const Block* block = level.GetBlockAt(leftTile, y);
            if (block && block->IsSolid()) {
                AABB blockBox = block->GetAABB();
                if (bowserBox.Intersects(blockBox)) {
                    hitWall = true;
                    break;
                }
            }
        }
    }

    if (hitWall) {
        m_PatrolDirection *= -1;
    }

    AABB footprint = state.GetCollider();
    footprint.bottom += GameConfig::GRAVITY_ACCELERATION;

    bool isGrounded = false;
    const std::vector<std::shared_ptr<Block>>& blocks = level.GetAllBlocks();
    for (const auto& block : blocks) {
        if (block && footprint.Intersects(block->GetAABB())) {
            isGrounded = true;
            break;
        }
    }

    state.SetGrounded(isGrounded);
    state.SetDirection(m_PatrolDirection > 0 ? 1 : 0);
}

void BowserBehavior::UpdateDamaged(EntityState& state) {
    m_DamageFlashCounter++;

    if (m_DamageFlashCounter >= DAMAGE_FLASH_DURATION) {
        // Damage phase complete - return to patrol
        m_Phase = BowserPhase::PATROL;
        m_PhaseTimer = 0;
        m_DamageFlashCounter = 0;
    }

    // Flashing effect handled by View layer (alternating sprite visibility)
}

void BowserBehavior::UpdateDefeated(EntityState& state) {
    // Apply gravity for falling into lava
    double fallHeight = state.GetFallHeight();
    double velY = state.GetVelY();
    float yDelta =
        PhysicsEngine::ApplyGravity(fallHeight, velY, false);  // Never grounded
    state.SetFallHeight(fallHeight);
    state.SetVelY(velY);
    state.SetWorldY(state.GetWorldY() + yDelta);

    // Once Bowser falls off screen, mark complete
    if (state.GetWorldY() > GameConfig::WINDOW_HEIGHT + 100) {
        state.Delete();
    }
}

bool BowserBehavior::OnPlayerCollision(EntityState& state, Player& player,
                                       bool isFromAbove) {
    // Player fireball hit Bowser
    if (m_HealthPoints > 0) {
        m_HealthPoints--;
        m_Phase = BowserPhase::DAMAGED;
        m_DamageFlashCounter = 0;

        if (m_HealthPoints <= 0) {
            m_Phase = BowserPhase::DEFEATED;
            state.SetFallHeight(0.0);  // Start falling
        }

        return true;  // Collision processed
    }

    // Bowser body collision damages player
    if (!isFromAbove &&
        player.GetState().GetPowerState() != PowerState::BIG_STAR &&
        player.GetState().GetPowerState() != PowerState::SMALL_STAR) {
        // Damage player (handled at App level)
        return true;
    }

    return false;
}

std::unique_ptr<IEntityBehavior> BowserBehavior::Clone() const {
    return std::make_unique<BowserBehavior>(*this);
}

}  // namespace Mario
