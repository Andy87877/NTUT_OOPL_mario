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
      m_PatrolDirection(-1),  // Start moving left (toward Mario)
      m_FireballPending(false),
      m_FireballX(0.0f),
      m_FireballY(0.0f),
      m_FireballDir(0) {}

void BowserBehavior::Update(EntityState& state, const Level& level,
                            const Player& player,
                            [[maybe_unused]] int gameTimer) {
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
            UpdateFireAttackPhase(state, player);
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
                                  [[maybe_unused]] const Player& player) {
    // NOTE: Physics (gravity + position update) is already applied by
    // App::UpdatePlaying() This method only handles AI patrol logic

    // Bowser moves slower than regular enemies, handled via VelX
    float moveSpeed = 3.0f;  // Slower than regular enemies
    state.SetVelX(moveSpeed * m_PatrolDirection);

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
    // Optimize: Check only blocks directly under Bowser's feet
    for (float ox : {0.1f, 0.5f, 0.9f}) {
        float checkX = bowserBox.left + (bowserBox.right - bowserBox.left) * ox;
        float checkY = bowserBox.bottom + 2.0f;
        const Block* b = level.GetBlockAtWorld(checkX, checkY);
        if (b && b->IsSolid()) {
            isGrounded = true;
            break;
        }
    }

    // Check for floor ahead to prevent walking off ledge
    if (isGrounded) {
        float edgeCheckX = (m_PatrolDirection > 0) ? bowserBox.right + 2.0f
                                                   : bowserBox.left - 2.0f;
        float edgeCheckY = bowserBox.bottom + 2.0f;
        const Block* edgeBlock = level.GetBlockAtWorld(edgeCheckX, edgeCheckY);
        if (!edgeBlock || !edgeBlock->IsSolid()) {
            m_PatrolDirection *= -1;  // No floor ahead — reverse
        }
    }

    state.SetGrounded(isGrounded);
    state.SetDirection(m_PatrolDirection > 0 ? 1 : 0);
}

void BowserBehavior::UpdateFireAttackPhase(const EntityState& state,
                                           const Player& player) {
    // Fire every ATTACK_INTERVAL ticks during the fire attack phase
    if (m_PhaseTimer % ATTACK_INTERVAL == 1) {
        Mario::AudioManager::GetInstance().PlaySFX(Mario::SFXName::EnemyFire);
        m_AttackCounter++;

        // Set pending fireball request — PlayingSceneHandler consumes it via
        // ConsumeSpawnRequest() on the same frame.
        m_FireballPending = true;

        // Determine direction toward player
        m_FireballDir = (player.GetWorldX() < state.GetWorldX()) ? 0 : 1;

        // Spawn fire OUTSIDE Bowser's body in the shooting direction so it
        // does not immediately overlap a wall tile or Bowser's own hitbox.
        if (m_FireballDir == 0) {
            // Shooting left: place 1 tile to the left of Bowser's left edge
            m_FireballX =
                state.GetWorldX() - static_cast<float>(GameConfig::TILE_SIZE);
        } else {
            // Shooting right: place at Bowser's right edge
            m_FireballX =
                state.GetWorldX() + static_cast<float>(state.GetWidth());
        }
        // Vertical: roughly the middle of Bowser's 2-tile body
        m_FireballY =
            state.GetWorldY() + static_cast<float>(GameConfig::TILE_SIZE);
    }
}

void BowserBehavior::UpdateJumpAttack(EntityState& state, const Level& level,
                                      [[maybe_unused]] const Player& player) {
    // NOTE: Physics (gravity + position update) is already applied by
    // App::UpdatePlaying()

    float moveSpeed = 5.0f;  // Faster during jump phase
    state.SetVelX(moveSpeed * m_PatrolDirection);

    // Jump randomly toward player
    if (m_PhaseTimer % 80 == 0 && state.IsGrounded()) {
        state.SetGrounded(false);
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
    for (float ox : {0.1f, 0.5f, 0.9f}) {
        float checkX = bowserBox.left + (bowserBox.right - bowserBox.left) * ox;
        float checkY = bowserBox.bottom + 2.0f;
        const Block* b = level.GetBlockAtWorld(checkX, checkY);
        if (b && b->IsSolid()) {
            isGrounded = true;
            break;
        }
    }

    state.SetGrounded(isGrounded);
    state.SetDirection(m_PatrolDirection > 0 ? 1 : 0);
}

void BowserBehavior::UpdateDamaged([[maybe_unused]] EntityState& state) {
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
    // NOTE: Physics is now handled globally. Defeated Bowser just falls
    // (Grounded = false).
    state.SetGrounded(false);

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

bool BowserBehavior::OnFireballHit(EntityState& state) {
    if (m_Phase == BowserPhase::DEFEATED || m_Phase == BowserPhase::DAMAGED)
        return true;  // Already in progress — absorb hit, don't re-trigger

    if (m_HealthPoints > 0) {
        m_HealthPoints--;
        m_Phase = BowserPhase::DAMAGED;
        m_DamageFlashCounter = 0;

        if (m_HealthPoints <= 0) {
            m_Phase = BowserPhase::DEFEATED;
            state.SetGravity(true);
            state.SetCollidable(false);
            AudioManager::GetInstance().PlaySFX(SFXName::BowserDie);
        } else {
            AudioManager::GetInstance().PlaySFX(SFXName::Kick);
        }
        return true;  // Tell CollisionManager: entity handled — only delete
                      // fireball
    }
    return false;
}

bool BowserBehavior::ConsumeSpawnRequest(EntityType& outType, float& outX,
                                         float& outY, int& outDir) {
    if (!m_FireballPending) return false;
    m_FireballPending = false;
    outType = EntityType::FIRE;  // Bowser fire projectile
    outX = m_FireballX;
    outY = m_FireballY;
    outDir = m_FireballDir;
    return true;
}

std::unique_ptr<IEntityBehavior> BowserBehavior::Clone() const {
    return std::make_unique<BowserBehavior>(*this);
}

}  // namespace Mario
