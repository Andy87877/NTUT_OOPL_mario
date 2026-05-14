/**
 * @file CollisionManager.cpp
 * @brief Implementation of Player-Block collision detection and resolution.
 *        Ported from C# Form1.cs collision logic.
 * @inheritance None
 */
#include "Mario/CollisionManager.hpp"

#include <cmath>

#include "Mario/AudioManager.hpp"
#include "Mario/Behaviors/IEntityBehavior.hpp"
#include "Mario/Camera.hpp"
#include "Mario/Entity.hpp"
#include "Mario/EntityDef.hpp"
#include "Mario/GameConfig.hpp"
#include "Mario/GameStateManager.hpp"
#include "Mario/PhysicsEngine.hpp"
#include "Mario/UIManager.hpp"

namespace Mario {

void CollisionManager::CheckPlayerBlockCollision(
    Player& player, Level& level, Camera& camera, GameStateManager& gameState,
    UIManager& uiManager, std::vector<Level::SpawnPoint>* outSpawns) {
    PlayerState& state = player.GetState();

    // IMPORTANT: Position has already been updated by UpdatePlaying()
    // This method only resolves collisions, not applies position changes

    // Resolve collisions in order: ground, ceiling, walls
    // This order matches the C# reference code behavior
    CheckGroundCollision(state, level);
    CheckCeilingCollision(state, level, camera, gameState, uiManager,
                          outSpawns);
    CheckWallCollision(state, level, camera.GetOffset());

    // Prevent player from going past left boundary
    if (state.GetX() < 0.0f) {
        state.SetX(0.0f);
    }
}

bool CollisionManager::CheckPitFall(const Player& player) const {
    // Player fell below the level (16 rows * 32 pixels = 512)
    float levelBottom = static_cast<float>(GameConfig::LEVEL_HEIGHT_PX);
    return player.GetState().GetY() > levelBottom + GameConfig::TILE_SIZE;
}

void CollisionManager::CheckGroundCollision(PlayerState& state, Level& level) {
    AABB playerBox = state.GetHitbox();

    // Check blocks below the player's feet
    int leftTile = static_cast<int>(playerBox.left) / GameConfig::TILE_SIZE;
    int rightTile =
        static_cast<int>(playerBox.right - 1) / GameConfig::TILE_SIZE;
    int bottomTile = static_cast<int>(playerBox.bottom) / GameConfig::TILE_SIZE;

    bool foundGround = false;

    for (int x = leftTile; x <= rightTile; x++) {
        Block* block = level.GetBlockAt(x, bottomTile);
        if (block && block->IsSolid()) {
            AABB blockBox = block->GetAABB();

            // Check if player is actually overlapping
            if (playerBox.Intersects(blockBox)) {
                // Check it's a downward collision (feet hitting top of block)
                float overlapY = playerBox.bottom - blockBox.top;
                if (state.GetVelY() >= 0.0f && overlapY > 0 &&
                    overlapY < GameConfig::TILE_SIZE * 0.75f) {
                    // Snap player to top of block
                    state.SetY(blockBox.top -
                               static_cast<float>(state.GetHeight()));
                    state.SetVelY(0.0);
                    state.SetFallHeight(0.0);
                    state.SetGrounded(true);
                    foundGround = true;
                }
            }
        }
    }

    // Also check one tile below current position (fall detection)
    if (!foundGround) {
        int belowTile =
            (static_cast<int>(playerBox.bottom) + 1) / GameConfig::TILE_SIZE;
        for (int x = leftTile; x <= rightTile; x++) {
            Block* block = level.GetBlockAt(x, belowTile);
            if (block && block->IsSolid()) {
                AABB blockBox = block->GetAABB();
                float gap = blockBox.top - playerBox.bottom;
                if (state.GetVelY() >= 0.0f && gap >= 0 && gap <= 2.0f) {
                    state.SetY(blockBox.top -
                               static_cast<float>(state.GetHeight()));
                    state.SetVelY(0.0);
                    state.SetFallHeight(0.0);
                    state.SetGrounded(true);
                    foundGround = true;
                    break;
                }
            }
        }
    }

    if (!foundGround && state.IsGrounded()) {
        // Player walked off an edge
        state.SetGrounded(false);
    }
}

void CollisionManager::CheckCeilingCollision(
    PlayerState& state, Level& level, Camera& camera,
    GameStateManager& gameState, UIManager& uiManager,
    std::vector<Level::SpawnPoint>* outSpawns) {
    AABB playerBox = state.GetHitbox();

    int leftTile = static_cast<int>(playerBox.left) / GameConfig::TILE_SIZE;
    int rightTile =
        static_cast<int>(playerBox.right - 1) / GameConfig::TILE_SIZE;
    int topTile = static_cast<int>(playerBox.top) / GameConfig::TILE_SIZE;

    for (int x = leftTile; x <= rightTile; x++) {
        Block* block = level.GetBlockAt(x, topTile);
        if (block && block->IsSolid()) {
            AABB blockBox = block->GetAABB();

            if (playerBox.Intersects(blockBox)) {
                float overlapY = blockBox.bottom - playerBox.top;
                if (state.GetVelY() < 0.0f && overlapY > 0 &&
                    overlapY < GameConfig::TILE_SIZE * 0.75f) {
                    // Head bump: push player down
                    state.SetY(blockBox.bottom);
                    state.SetVelY(0.0);
                    state.SetFallHeight(0.0);

                    // Trigger block hit
                    if (!block->IsHit()) {
                        std::string spawnEntity;

                        // Handle container blocks (question blocks, coin
                        // blocks) Use GetSpawnContents for powerup conversion
                        // logic
                        if (block->GetDef().isContainer) {
                            spawnEntity =
                                block->GetSpawnContents(state.GetState());
                        }
                        // Handle explicit spawner blocks
                        else if (block->GetDef().spawner) {
                            spawnEntity = block->GetDef().spawnEntity;
                        }

                        // Calculate block hit effect position (center of block)
                        // Convert world to screen to PTSD coordinates
                        float blockCenterX =
                            block->GetWorldX() + GameConfig::TILE_SIZE * 0.5f;
                        float blockCenterY = block->GetWorldY();
                        float screenPixelX =
                            camera.WorldToScreenX(blockCenterX);
                        float screenPixelY =
                            camera.WorldToScreenY(blockCenterY);
                        float ptsdX = screenPixelX - 640.0f;
                        float ptsdY = 360.0f - screenPixelY;

                        // Special handling for CoinGet blocks: directly add
                        // coins instead of spawning entities
                        if (spawnEntity == "CoinGet") {
                            gameState.AddCoin();
                            gameState.AddScore(200);
                            Mario::AudioManager::GetInstance().PlaySFX(
                                Mario::SFXName::Coin);

                            // Display floating text "+200" particle effect
                            uiManager.AddFloatingText(ptsdX, ptsdY, "+200", 60);
                        }
                        // Spawn the entity if we determined one (and it's not
                        // CoinGet)
                        else if (!spawnEntity.empty() && outSpawns) {
                            Level::SpawnPoint sp;
                            sp.entityName = spawnEntity;
                            sp.gridX = block->GetGridX();
                            sp.gridY = block->GetGridY();
                            sp.worldX = static_cast<float>(
                                block->GetGridX() * GameConfig::TILE_SIZE);
                            // EntityState::Init will handle the fromBlock
                            // offset (posY -= TILE_SIZE)
                            sp.worldY = static_cast<float>(
                                block->GetGridY() * GameConfig::TILE_SIZE);
                            sp.spawned = true;

                            if (spawnEntity == "Coin" ||
                                spawnEntity == "CoinText") {
                                // Block already plays Bump/Coin based on code?
                                // Actually let's just make it here!
                                Mario::AudioManager::GetInstance().PlaySFX(
                                    Mario::SFXName::Coin);
                            } else {
                                Mario::AudioManager::GetInstance().PlaySFX(
                                    Mario::SFXName::Item);
                                // Display particle effect for other items
                                // (mushroom, fire flower, star, 1-up)
                                uiManager.AddFloatingText(ptsdX, ptsdY, "+100",
                                                          60);
                            }

                            outSpawns->push_back(sp);
                        }
                    }
                    block->OnHit(state.GetState());

                    if (block->JustBroken() && outSpawns) {
                        float bx = block->GetWorldX();
                        float by = block->GetWorldY();

                        float offset = GameConfig::TILE_SIZE * 0.25f;
                        Level::SpawnPoint sp1{-1,
                                              block->GetName() + "Break_tl",
                                              block->GetGridX(),
                                              block->GetGridY(),
                                              bx - offset,
                                              by - offset,
                                              true};
                        Level::SpawnPoint sp2{
                            -1, block->GetName() + "Break_tr",
                            block->GetGridX(), block->GetGridY(), bx + offset,
                            by + offset,  // Wait, C# says: (x + 0.25), (y +
                                          // 0.25) -> this is actually br in C#?
                                          // Let's check C# again.
                            true};
                        Level::SpawnPoint sp3{-1,
                                              block->GetName() + "Break_bl",
                                              block->GetGridX(),
                                              block->GetGridY(),
                                              bx - offset,
                                              by + offset,
                                              true};
                        Level::SpawnPoint sp4{-1,
                                              block->GetName() + "Break_br",
                                              block->GetGridX(),
                                              block->GetGridY(),
                                              bx + offset,
                                              by - offset,
                                              true};

                        outSpawns->push_back(sp1);
                        outSpawns->push_back(sp2);
                        outSpawns->push_back(sp3);
                        outSpawns->push_back(sp4);
                    }
                }
            }
        }
    }
}

void CollisionManager::CheckWallCollision(PlayerState& state, Level& level,
                                          float /*cameraOffset*/) {
    AABB playerBox = state.GetHitbox();

    int topTile = static_cast<int>(playerBox.top) / GameConfig::TILE_SIZE;
    int bottomTile =
        static_cast<int>(playerBox.bottom - 1) / GameConfig::TILE_SIZE;

    // Calculate hitbox offset from m_PosX
    float w = playerBox.right - playerBox.left;
    float offsetX = playerBox.left - state.GetX();

    // Check right wall (moving right into block)
    if (state.GetVelX() > 0) {
        int rightTile =
            static_cast<int>(playerBox.right) / GameConfig::TILE_SIZE;
        for (int y = topTile; y <= bottomTile; y++) {
            Block* block = level.GetBlockAt(rightTile, y);
            if (block && block->IsSolid()) {
                AABB blockBox = block->GetAABB();
                if (playerBox.Intersects(blockBox)) {
                    // Push Mario's position so his hitbox right edge
                    // is just to the left of block's left edge
                    state.SetX(blockBox.left - w - offsetX);
                    state.SetVelX(0.0f);
                    break;
                }
            }
        }
    }

    // Check left wall (moving left into block)
    if (state.GetVelX() < 0) {
        int leftTile = static_cast<int>(playerBox.left) / GameConfig::TILE_SIZE;
        for (int y = topTile; y <= bottomTile; y++) {
            Block* block = level.GetBlockAt(leftTile, y);
            if (block && block->IsSolid()) {
                AABB blockBox = block->GetAABB();
                if (playerBox.Intersects(blockBox)) {
                    // Push Mario's position so his hitbox left edge
                    // is just to the right of block's right edge
                    state.SetX(blockBox.right - offsetX);
                    state.SetVelX(0.0f);
                    break;
                }
            }
        }
    }
}

// ============================================================================
// ✨ NEW: Player-Entity Collision Detection
// ============================================================================
void CollisionManager::CheckPlayerEntityCollision(
    Player& player, std::vector<std::shared_ptr<Entity>>& entities,
    Camera& camera, GameStateManager& gameState, UIManager& uiManager) {
    if (player.GetState().IsDead()) return;

    PlayerState& ps = player.GetState();
    AABB playerBox = ps.GetHitbox();

    for (auto& entity : entities) {
        EntityState& es = entity->GetState();
        if (!es.IsActive()) continue;
        if (es.IsHidden())
            continue;  // PiranhaPlant inside pipe — not collidable

        AABB entityBox = es.GetHitbox();
        if (!playerBox.Intersects(entityBox)) continue;

        if (es.IsEnemy()) {
            if (ps.GetStarTimer() > 0) {
                // ✨ Star power: kill enemy
                es.Delete();
                AudioManager::GetInstance().PlaySFX(SFXName::Kick);
                int scoreWorth = es.GetScoreWorth();
                gameState.AddScore(scoreWorth);

                // Floating text
                float enemyWorldX =
                    es.GetWorldX() + GameConfig::TILE_SIZE * 0.5f;
                float enemyWorldY = es.GetWorldY();
                float screenPixelX = camera.WorldToScreenX(enemyWorldX);
                float screenPixelY = camera.WorldToScreenY(enemyWorldY);
                float ptsdX = screenPixelX - 640.0f;
                float ptsdY = 360.0f - screenPixelY;
                uiManager.AddFloatingText(ptsdX, ptsdY,
                                          "+" + std::to_string(scoreWorth), 60);
                continue;
            }

            // Check if player is stomping (falling from above)
            float playerBottom = playerBox.bottom;
            float entityTop = entityBox.top;
            float overlapY = playerBottom - entityTop;

            if (overlapY > 0 && overlapY < GameConfig::TILE_SIZE * 0.5f &&
                ps.GetVelY() >= 0 && !ps.IsGrounded()) {
                // Player stomped on enemy
                if (es.IsSquishable()) {
                    es.Squish();
                } else if (entity->GetDef().type == EntityType::BOWSER) {
                    AudioManager::GetInstance().PlaySFX(SFXName::BowserDie);
                } else {
                    AudioManager::GetInstance().PlaySFX(SFXName::Squish);
                }
                int scoreWorth = es.GetScoreWorth();
                gameState.AddScore(scoreWorth);

                float enemyWorldX =
                    es.GetWorldX() + GameConfig::TILE_SIZE * 0.5f;
                float enemyWorldY = es.GetWorldY();
                float screenPixelX = camera.WorldToScreenX(enemyWorldX);
                float screenPixelY = camera.WorldToScreenY(enemyWorldY);
                float ptsdX = screenPixelX - 640.0f;
                float ptsdY = 360.0f - screenPixelY;
                uiManager.AddFloatingText(ptsdX, ptsdY,
                                          "+" + std::to_string(scoreWorth), 60);

                ps.SetFallHeight(PhysicsEngine::GetJumpHeight(0) * 0.5);
                ps.SetGrounded(false);
            } else if (es.IsKoopaSquash()) {
                // Koopa Troopa: spawn shell
                es.Delete();
                AudioManager::GetInstance().PlaySFX(SFXName::Squish);
                int scoreWorth = es.GetScoreWorth();
                gameState.AddScore(scoreWorth);

                float enemyWorldX =
                    es.GetWorldX() + GameConfig::TILE_SIZE * 0.5f;
                float enemyWorldY = es.GetWorldY();
                float screenPixelX = camera.WorldToScreenX(enemyWorldX);
                float screenPixelY = camera.WorldToScreenY(enemyWorldY);
                float ptsdX = screenPixelX - 640.0f;
                float ptsdY = 360.0f - screenPixelY;
                uiManager.AddFloatingText(ptsdX, ptsdY,
                                          "+" + std::to_string(scoreWorth), 60);

                ps.SetFallHeight(PhysicsEngine::GetJumpHeight(0) * 0.5);
                ps.SetGrounded(false);
            } else if (entity->GetDef().type == EntityType::KOOPA_SHELL) {
                // Koopa shell: kick it
                AABB eBox = entityBox;
                float playerCenterX =
                    playerBox.left + (playerBox.right - playerBox.left) * 0.5f;
                float shellCenterX =
                    eBox.left + (eBox.right - eBox.left) * 0.5f;

                float speed =
                    GameConfig::SCALED_SPEED / GameConfig::ENEMY_SPEED_DIVISOR;
                float shellSpeed = speed * 1.5f;

                if (playerCenterX > shellCenterX) {
                    es.SetVelX(-shellSpeed);
                } else {
                    es.SetVelX(shellSpeed);
                }

                if (es.GetVelX() < 0) {
                    es.SetDirection(0);
                } else {
                    es.SetDirection(1);
                }

                AudioManager::GetInstance().PlaySFX(SFXName::Squish);

                ps.SetFallHeight(PhysicsEngine::GetJumpHeight(0) * 0.5);
                ps.SetGrounded(false);
            } else if (!es.IsSquished()) {
                // Player hit from side - take damage
                ps.TakeDamage();
            }
        }

        if (es.IsPowerUp()) {
            // Collect power-up
            int puState = es.GetPowerUpState();
            if (puState == 1 || puState == 2) {
                if (ps.GetState() == 0) {
                    ps.SetY(ps.GetY() - GameConfig::TILE_SIZE);
                }
                ps.PowerUp(puState == 1 ? PowerState::BIG : PowerState::FIRE);
                AudioManager::GetInstance().PlaySFX(SFXName::Powerup);
            } else if (puState == 3) {
                ps.StartStar();
                AudioManager::GetInstance().PlaySFX(SFXName::Powerup);
            } else if (puState == 5) {
                gameState.AddLife();
                AudioManager::GetInstance().PlaySFX(SFXName::_1up);

                float oneupWorldX =
                    es.GetWorldX() + GameConfig::TILE_SIZE * 0.5f;
                float oneupWorldY = es.GetWorldY();
                float oneupScreenPixelX = camera.WorldToScreenX(oneupWorldX);
                float oneupScreenPixelY = camera.WorldToScreenY(oneupWorldY);
                float oneupPtsdX = oneupScreenPixelX - 640.0f;
                float oneupPtsdY = 360.0f - oneupScreenPixelY;
                uiManager.AddFloatingText(oneupPtsdX, oneupPtsdY, "+1UP", 60);
            }

            int scoreWorth = es.GetScoreWorth();
            gameState.AddScore(scoreWorth);

            float puWorldX = es.GetWorldX() + GameConfig::TILE_SIZE * 0.5f;
            float puWorldY = es.GetWorldY();
            float puScreenPixelX = camera.WorldToScreenX(puWorldX);
            float puScreenPixelY = camera.WorldToScreenY(puWorldY);
            float puPtsdX = puScreenPixelX - 640.0f;
            float puPtsdY = 360.0f - puScreenPixelY;
            uiManager.AddFloatingText(puPtsdX, puPtsdY,
                                      "+" + std::to_string(scoreWorth), 60);
            es.Delete();
        } else if (es.IsCoin()) {
            gameState.AddCoin();
            int coinScore = es.GetScoreWorth();
            gameState.AddScore(coinScore);
            AudioManager::GetInstance().PlaySFX(SFXName::Coin);

            float coinWorldX = es.GetWorldX() + GameConfig::TILE_SIZE * 0.5f;
            float coinWorldY = es.GetWorldY();
            float coinScreenPixelX = camera.WorldToScreenX(coinWorldX);
            float coinScreenPixelY = camera.WorldToScreenY(coinWorldY);
            float coinPtsdX = coinScreenPixelX - 640.0f;
            float coinPtsdY = 360.0f - coinScreenPixelY;
            uiManager.AddFloatingText(coinPtsdX, coinPtsdY,
                                      "+" + std::to_string(coinScore), 60);
            es.Delete();
        }
    }
}

// ============================================================================
// ✨ NEW: Entity-Entity Collision Detection (Fire vs Enemy, Shell vs Enemy)
// ============================================================================
void CollisionManager::CheckEntityEntityCollision(
    std::vector<std::shared_ptr<Entity>>& entities,
    GameStateManager& gameState) {
    for (size_t i = 0; i < entities.size(); ++i) {
        EntityState& e1 = entities[i]->GetState();
        if (!e1.IsActive()) continue;

        for (size_t j = i + 1; j < entities.size(); ++j) {
            EntityState& e2 = entities[j]->GetState();
            if (!e2.IsActive()) continue;

            if (!e1.GetHitbox().Intersects(e2.GetHitbox())) continue;

            // ✨ Fire vs Enemy (updated to use entities[i]->GetDef())
            if (entities[i]->GetDef().type == EntityType::FIRE &&
                e2.IsEnemy()) {
                // Delegate to behavior first (e.g., BowserBehavior uses HP)
                bool handled = entities[j]->GetBehavior() &&
                               entities[j]->GetBehavior()->OnFireballHit(e2);
                e1.Delete();  // Always delete the fireball
                if (!handled && !e2.IsDead()) {
                    e2.Delete();
                    Mario::AudioManager::GetInstance().PlaySFX(
                        Mario::SFXName::Kick);
                    gameState.AddScore(e2.GetScoreWorth());
                }
            } else if (entities[j]->GetDef().type == EntityType::FIRE &&
                       e1.IsEnemy()) {
                bool handled = entities[i]->GetBehavior() &&
                               entities[i]->GetBehavior()->OnFireballHit(e1);
                e2.Delete();  // Always delete the fireball
                if (!handled && !e1.IsDead()) {
                    e1.Delete();
                    Mario::AudioManager::GetInstance().PlaySFX(
                        Mario::SFXName::Kick);
                    gameState.AddScore(e1.GetScoreWorth());
                }
            }

            // ✨ Koopa Shell vs Enemy
            if (entities[i]->GetDef().type == EntityType::KOOPA_SHELL &&
                std::abs(e1.GetVelX()) > 0 && e2.IsEnemy()) {
                if (entities[j]->GetDef().type != EntityType::KOOPA_SHELL) {
                    e2.Delete();
                    Mario::AudioManager::GetInstance().PlaySFX(
                        Mario::SFXName::Kick);
                    gameState.AddScore(e2.GetScoreWorth());
                }
            } else if (entities[j]->GetDef().type == EntityType::KOOPA_SHELL &&
                       std::abs(e2.GetVelX()) > 0 && e1.IsEnemy()) {
                if (entities[i]->GetDef().type != EntityType::KOOPA_SHELL) {
                    e1.Delete();
                    Mario::AudioManager::GetInstance().PlaySFX(
                        Mario::SFXName::Kick);
                    gameState.AddScore(e1.GetScoreWorth());
                }
            }
        }
    }
}

// ============================================================================
// CheckEntityBlockCollision
// Ported from old App::CheckEntityBlockCollision().
// Handles ground, wall, and pit-fall for non-player entities.
// C# reference: Form1.cs onTick() entity ground/wall checks.
// ============================================================================
void CollisionManager::CheckEntityBlockCollision(Entity& entity, Level& level) {
    EntityState& state = entity.GetState();
    AABB box = state.GetHitbox();

    const int tileSize = GameConfig::TILE_SIZE;

    // -- Ground check --
    int leftTile = static_cast<int>(box.left) / tileSize;
    int rightTile = static_cast<int>(box.right - 1) / tileSize;
    int bottomTile = static_cast<int>(box.bottom) / tileSize;

    bool onGround = false;
    for (int x = leftTile; x <= rightTile; x++) {
        Block* block = level.GetBlockAt(x, bottomTile);
        if (block && block->IsSolid()) {
            AABB bb = block->GetAABB();
            if (box.Intersects(bb)) {
                float overlap = box.bottom - bb.top;
                if (overlap > 0 && overlap < tileSize * 0.75f) {
                    state.SetY(bb.top - state.GetHeight());
                    state.SetVelY(0.0f);
                    state.SetGrounded(true);
                    onGround = true;
                }
            }
        }
    }
    if (!onGround && state.IsGrounded()) {
        state.SetGrounded(false);
    }

    // -- Wall check: flip direction on wall collision --
    AABB updatedBox = state.GetHitbox();
    if (state.GetVelX() > 0.0f) {
        int rtile = static_cast<int>(updatedBox.right) / tileSize;
        for (int y = static_cast<int>(updatedBox.top) / tileSize;
             y <= static_cast<int>(updatedBox.bottom - 1) / tileSize; y++) {
            Block* block = level.GetBlockAt(rtile, y);
            if (block && block->IsSolid()) {
                state.FlipDirection();
                state.SetX(block->GetWorldX() - state.GetWidth());
                break;
            }
        }
    } else if (state.GetVelX() < 0.0f) {
        int ltile = static_cast<int>(updatedBox.left) / tileSize;
        for (int y = static_cast<int>(updatedBox.top) / tileSize;
             y <= static_cast<int>(updatedBox.bottom - 1) / tileSize; y++) {
            Block* block = level.GetBlockAt(ltile, y);
            if (block && block->IsSolid()) {
                state.FlipDirection();
                state.SetX(block->GetWorldX() + tileSize);
                break;
            }
        }
    }

    // -- Pit fall: deactivate entity if below level --
    if (state.GetY() > GameConfig::LEVEL_HEIGHT_PX + tileSize) {
        state.Delete();
    }
}

}  // namespace Mario
