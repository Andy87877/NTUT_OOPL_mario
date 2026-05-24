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
      m_AxeThrowTimer(0),
      m_FireballTimer(FIREBALL_INTERVAL),
      m_BridgeLeft(-1.0f),
      m_BridgeRight(-1.0f) {}

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

    // Dynamic Bridge Scanning (zero-allocation lookup)
    if (m_BridgeLeft < 0.0f) {
        float left = 999999.0f;
        float right = -999999.0f;
        for (const auto& block : level.GetAllBlocks()) {
            if (block && (block->GetName() == "Bridge" || block->GetName() == "BridgeBlock" || block->GetBlockID() == 818)) {
                float bx = block->GetWorldX();
                if (bx > 12000.0f) { // Bowser's bridge is only in Room 5
                    if (bx < left) left = bx;
                    if (bx > right) right = bx;
                }
            }
        }
        if (left < right) {
            m_BridgeLeft = left;
            m_BridgeRight = right + static_cast<float>(GameConfig::TILE_SIZE);
        } else {
            // Fallback: standard bridge dimensions in 8-4 Room 5
            m_BridgeLeft = 14500.0f;
            m_BridgeRight = 15300.0f;
        }
    }

    // Intelligent Fireball Dodge Hop & Counter-Attack
    bool isMarioShooting = player.GetState().IsFireShooting();
    float distanceToMario = std::abs(player.GetWorldX() - state.GetWorldX());
    if (isMarioShooting && distanceToMario < 350.0f && state.IsGrounded()) {
        state.SetGrounded(false);
        state.SetFallHeight(GameConfig::JUMP_LOW_VELOCITY * 1.1f); // Quick evasive hop
        m_AxeThrowTimer = AXE_THROW_INTERVAL; // Counter-attack with axe immediately!
        LOG_INFO("Bowser: Dodging incoming fireball with counter-attack!");
    }

    // Axe throwing logic: active during patrol, fire attack, and jump phases
    m_AxeThrowTimer++;
    if (m_AxeThrowTimer >= AXE_THROW_INTERVAL) {
        m_AxeThrowTimer = 0;
        // Spawn axe: slightly above Bowser's head
        float axeX = state.GetWorldX() + (state.GetDirection() == 0 ? -10.0f : state.GetWidth() - 10.0f);
        float axeY = state.GetWorldY() - 15.0f; // 15px above head
        int axeDir = (player.GetWorldX() < state.GetWorldX()) ? 0 : 1;
        m_PendingSpawns.push_back({EntityType::AXE_PROJECTILE, axeX, axeY, axeDir});
    }

    // Fire spitting logic: active during patrol, fire attack, and jump phases
    m_FireballTimer++;
    if (m_FireballTimer >= FIREBALL_INTERVAL) {
        m_FireballTimer = 0;
        Mario::AudioManager::GetInstance().PlaySFX(Mario::SFXName::EnemyFire);

        // Determine direction toward player (always left 0 if player is to his left)
        int fireballDir = (player.GetWorldX() < state.GetWorldX()) ? 0 : 1;

        float fireballX = 0.0f;
        if (fireballDir == 0) {
            fireballX = state.GetWorldX() - static_cast<float>(GameConfig::TILE_SIZE);
        } else {
            fireballX = state.GetWorldX() + static_cast<float>(state.GetWidth());
        }

        // Intelligent Targeted Fire spits at height matching Mario
        float marioY = player.GetWorldY();
        float fireballY = state.GetWorldY();
        
        if (marioY < state.GetWorldY() - GameConfig::TILE_SIZE * 0.5f) {
            fireballY += 0.2f * GameConfig::TILE_SIZE; // High fire to intercept jumping Mario
        } else if (marioY > state.GetWorldY() + GameConfig::TILE_SIZE * 0.5f || player.GetState().IsCrouching()) {
            fireballY += 1.8f * GameConfig::TILE_SIZE; // Low fire to hit crouching or grounded Mario
        } else {
            fireballY += 1.0f * GameConfig::TILE_SIZE; // Medium fire to catch normal height Mario
        }

        m_PendingSpawns.push_back({EntityType::FIRE, fireballX, fireballY, fireballDir});
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

    // Intelligent Blocking & Interception Algorithm
    float dist = std::abs(player.GetWorldX() - state.GetWorldX());
    bool playerTryingToPass = (player.GetWorldX() > state.GetWorldX());
    bool playerJumpingHigh = (player.GetWorldY() < state.GetWorldY() - GameConfig::TILE_SIZE * 2.0f);

    if (dist < 220.0f && state.IsGrounded()) {
        if (playerJumpingHigh || playerTryingToPass) {
            // Jump high to physically block/intercept Mario!
            state.SetGrounded(false);
            state.SetFallHeight(GameConfig::JUMP_HIGH_VELOCITY * 0.85f); // High blocking jump
            
            // If player is trying to pass to the right, face right and jump right to block!
            if (playerTryingToPass) {
                m_PatrolDirection = 1;
                state.SetVelX(5.0f); // Move fast to intercept
            } else {
                m_PatrolDirection = -1;
                state.SetVelX(-3.0f);
            }
            LOG_INFO("Bowser: Intercepting and blocking player jump/passage!");
        }
    }

    // Keep Bowser strictly constrained on the bridge Y boundaries
    if (m_BridgeLeft > 0.0f) {
        float minX = m_BridgeLeft + 10.0f;
        float maxX = m_BridgeRight - static_cast<float>(state.GetWidth()) - 10.0f;
        if (state.GetX() < minX) {
            state.SetX(minX);
            m_PatrolDirection = 1;
            state.SetVelX(std::abs(state.GetVelX()));
        } else if (state.GetX() > maxX) {
            state.SetX(maxX);
            m_PatrolDirection = -1;
            state.SetVelX(-std::abs(state.GetVelX()));
        }
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
    // Fire spitting is now handled continuously in Update() to enable
    // off-screen spitting right from the start of the level.
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
    if (m_PendingSpawns.empty()) return false;
    auto req = m_PendingSpawns.front();
    m_PendingSpawns.erase(m_PendingSpawns.begin());
    outType = req.type;
    outX = req.x;
    outY = req.y;
    outDir = req.dir;
    return true;
}

std::unique_ptr<IEntityBehavior> BowserBehavior::Clone() const {
    return std::make_unique<BowserBehavior>(*this);
}

}  // namespace Mario
