/**
 * @file CollisionManager.cpp
 * @brief Implementation of Player-Block collision detection and resolution.
 *        Fully ported from C# Form1.cs onTick() collision pipeline.
 *        Pipeline: FallDetect → ceiling trigger → per-block resolution.
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

// ============================================================================
// File-scope helper: build the full-body AABB (C# GetRecPosition equivalent).
// The narrow hitbox (HITBOX_WIDTH_RATIO) is only used for the ceiling trigger
// (Step 2). All other collision checks use the full TILE_SIZE width to match
// C# which uses GetRecPosition() in its main collision loop.
// ============================================================================
static AABB BodyRect(const PlayerState& state) {
    return AABB::FromPosSize(state.GetX(), state.GetY(),
                             static_cast<float>(GameConfig::TILE_SIZE),
                             static_cast<float>(state.GetHeight()));
}

// ============================================================================
// CheckPlayerBlockCollision
// Exactly matches the C# Form1.cs onTick() collision pipeline:
//
//   Step 1 — FallDetect (C# "marioIntersect" check)
//     Thin 4px strip below feet. No solid block → SetGrounded(false).
//
//   Step 2 — Ceiling trigger (uses narrow hitbox, C# GetHitBox)
//     Detects head-bump against block bottom while jumping upward.
//     Snaps position AND triggers block contents (questions, bricks).
//     Runs BEFORE the main loop so it uses the exact C# narrower hitbox.
//
//   Step 3 — Per-block resolution loop (uses full body rect, C# GetRecPosition)
//     For each nearby solid block that overlaps Mario's body rectangle:
//       Airborne  : DOWN → RIGHT → LEFT → DOWN (again) → UP → LEFT (again)
//       Grounded  : RIGHT or LEFT only
//     After each resolve step the body rect is recomputed from state.
// ============================================================================
void CollisionManager::CheckPlayerBlockCollision(
    Player& player, Level& level, Camera& camera, GameStateManager& gameState,
    UIManager& uiManager, std::vector<Level::SpawnPoint>* outSpawns) {
    PlayerState& state = player.GetState();

    const float TS = static_cast<float>(GameConfig::TILE_SIZE);
    const float STR = GameConfig::INTERSECT_STRICTNESS;  // 0.75f

    // Direction flags — match C# movingDown / movingUp / movingLeft /
    // movingRight. movingDown: airborne AND jump arc exhausted (falling phase).
    // movingUp  : jump arc still active (rising phase, fallHeight > 0).
    bool movingDown = !state.IsGrounded() && state.GetFallHeight() <= 0.0;
    bool movingUp = state.GetFallHeight() > 0.0;
    bool movingRight = state.GetVelX() > 0.0f;
    bool movingLeft = state.GetVelX() < 0.0f;

    // Tile-range anchor for spatial lookup (±2 columns, +4 rows covers any
    // state)
    AABB body = BodyRect(state);
    int tileX = static_cast<int>(body.left) / GameConfig::TILE_SIZE;
    int tileY = static_cast<int>(body.top) / GameConfig::TILE_SIZE;

    // -------------------------------------------------------------------------
    // Step 1: FallDetect (C# marioIntersect)
    // A full rectangle of sizeX by sizeY shifted down by 1 pixel.
    // Matches C# Player.cs fallDetect definition: new Rectangle(posX, posY + 1, sizeX, sizeY).
    // -------------------------------------------------------------------------
    AABB fallDetect = {body.left, body.top + 1.0f, body.right, body.bottom + 1.0f};
    bool groundFound = false;

    for (int gy = tileY; gy <= tileY + 3 && !groundFound; gy++) {
        for (int gx = tileX - 1; gx <= tileX + 2 && !groundFound; gx++) {
            Block* blk = level.GetBlockAt(gx, gy);
            if (blk && blk->IsSolid() &&
                blk->GetAABB().Intersects(fallDetect)) {
                groundFound = true;
            }
        }
    }
    for (auto* plat : level.GetMovingPlatforms()) {
        if (!groundFound && plat && plat->IsSolid() &&
            plat->GetAABB().Intersects(fallDetect)) {
            groundFound = true;
        }
    }
    if (!groundFound) {
        state.SetGrounded(false);
    }

    // -------------------------------------------------------------------------
    // Step 2: Ceiling trigger (narrow hitbox, C# GetHitBox)
    // Fires only while Mario is rising (movingUp). Uses the narrow hitbox to
    // detect when the head enters the bottom 75% of a block above.
    // Snaps the Y position and calls TriggerBlockHit which handles contents,
    // audio, and scoring. The "checkRow-1" extra pass handles the edge case
    // where Mario's head top is exactly on a tile boundary.
    // -------------------------------------------------------------------------
    if (movingUp) {
        AABB hb = state.GetHitbox();  // narrow hitbox for head detection
        int htLeft = static_cast<int>(hb.left) / GameConfig::TILE_SIZE;
        int htRight = static_cast<int>(hb.right) / GameConfig::TILE_SIZE;
        int headRow = static_cast<int>(hb.top) / GameConfig::TILE_SIZE;

        bool triggered = false;
        // Check current head tile row AND the one above (handles tile-boundary)
        for (int row = headRow - 1; row <= headRow && !triggered; row++) {
            for (int gx = htLeft; gx <= htRight && !triggered; gx++) {
                Block* blk = level.GetBlockAt(gx, row);
                if (!blk || !blk->IsSolid()) continue;
                AABB bb = blk->GetAABB();

                // C#: Top < b.Bottom && Top > b.Bottom - size*0.75 &&
                // intersects
                if (hb.top < bb.bottom && hb.top > bb.bottom - TS * STR &&
                    hb.Intersects(bb)) {
                    state.SetY(bb.bottom);  // snap head to block bottom
                    state.SetFallHeight(0.0);
                    state.SetVelY(0.0);
                    movingUp = false;

                    TriggerBlockHit(*blk, state, camera, gameState, uiManager,
                                    outSpawns);
                    triggered = true;
                }
            }
        }
    }

    // -------------------------------------------------------------------------
    // Step 3: Main per-block collision loop
    // A lambda handles one block so the same logic applies to both static
    // blocks and moving platforms.
    // -------------------------------------------------------------------------
    auto processBlock = [&](const AABB& bb) {
        body = BodyRect(state);
        if (!body.Intersects(bb)) return;

        // Reset movingRight/movingLeft for each block to match C# logic
        bool localMovingRight = movingRight;
        bool localMovingLeft = movingLeft;

        if (!state.IsGrounded()) {
            // Airborne resolution order (directly from C# Form1.cs):
            // DOWN → RIGHT → LEFT → DOWN (2nd pass) → UP → LEFT (2nd pass)
            ResolveDown(state, bb, movingDown);

            body = BodyRect(state);
            if (localMovingRight && body.Intersects(bb)) {
                ResolveRight(state, bb, localMovingRight);
            }

            body = BodyRect(state);
            if (localMovingLeft && body.Intersects(bb)) {
                ResolveLeft(state, bb, localMovingLeft);
            }

            // Second DOWN pass handles corner cases where a horizontal snap
            // places Mario back onto the block top.
            body = BodyRect(state);
            if (body.Intersects(bb)) {
                ResolveDown(state, bb, movingDown);
            }

            // UP (ceiling snap without content trigger — Step 2 already did
            // that)
            body = BodyRect(state);
            if (body.Intersects(bb)) {
                ResolveUp(state, bb, movingUp);
            }

            // Second LEFT pass (C# repeats this after UP)
            body = BodyRect(state);
            if (localMovingLeft && body.Intersects(bb)) {
                ResolveLeft(state, bb, localMovingLeft);
            }

        } else {
            // Grounded: horizontal resolution only (C# grounded branch)
            if (localMovingRight) {
                ResolveRight(state, bb, localMovingRight);
            } else if (localMovingLeft) {
                ResolveLeft(state, bb, localMovingLeft);
            }
        }
    };

    // Static blocks in the search window
    for (int gy = tileY - 1; gy <= tileY + 3; gy++) {
        for (int gx = tileX - 1; gx <= tileX + 2; gx++) {
            Block* blk = level.GetBlockAt(gx, gy);
            if (blk && blk->IsSolid()) {
                processBlock(blk->GetAABB());
            }
        }
    }

    // Moving platforms (same per-block logic)
    for (auto* plat : level.GetMovingPlatforms()) {
        if (plat && plat->IsSolid()) {
            processBlock(plat->GetAABB());
        }
    }

    // Clamp to left level boundary
    if (state.GetX() < 0.0f) {
        state.SetX(0.0f);
    }
}

// ============================================================================
// CheckPitFall
// ============================================================================
bool CollisionManager::CheckPitFall(const Player& player) const {
    float levelBottom = static_cast<float>(GameConfig::LEVEL_HEIGHT_PX);
    return player.GetState().GetY() > levelBottom + GameConfig::TILE_SIZE;
}

// ============================================================================
// Resolution helpers
// Each method mirrors one C# CheckCollisionsXxx function.
// All use BodyRect (full TILE_SIZE width) matching C# GetRecPosition().
// ============================================================================

// Snap feet to block top when falling (C# CheckCollisionsDown).
// C#: if (Bottom > b.Top && Bottom < b.Top + size*0.75 && movingDown)
void CollisionManager::ResolveDown(PlayerState& state, const AABB& bb,
                                   bool& movingDown) {
    AABB body = BodyRect(state);
    if (body.bottom > bb.top &&
        body.bottom < bb.top + static_cast<float>(GameConfig::TILE_SIZE) *
                                   GameConfig::INTERSECT_STRICTNESS &&
        movingDown) {
        state.SetY(bb.top - static_cast<float>(state.GetHeight()));
        state.SetGrounded(true);
        state.SetVelY(0.0);
        state.SetFallHeight(0.0);
        movingDown = false;
    }
}

// Snap head to block bottom while rising (C# CheckCollisionsUp).
// Content triggering is done in Step 2; this only resolves position.
// C#: if (Top < b.Bottom && Top > b.Bottom - size*0.75 && movingUp)
void CollisionManager::ResolveUp(PlayerState& state, const AABB& bb,
                                 bool& movingUp) {
    AABB body = BodyRect(state);
    if (body.top < bb.bottom &&
        body.top > bb.bottom - static_cast<float>(GameConfig::TILE_SIZE) *
                                   GameConfig::INTERSECT_STRICTNESS &&
        movingUp) {
        state.SetY(bb.bottom);
        state.SetFallHeight(0.0);
        state.SetVelY(0.0);
        movingUp = false;
    }
}

// Snap right edge to block left when moving right (C# CheckCollisionsRight).
// C#: if (Right > b.Left && Right < b.Left + size*0.75 && movingRight)
void CollisionManager::ResolveRight(PlayerState& state, const AABB& bb,
                                    bool& movingRight) {
    AABB body = BodyRect(state);
    if (body.right > bb.left &&
        body.right < bb.left + static_cast<float>(GameConfig::TILE_SIZE) *
                                   GameConfig::INTERSECT_STRICTNESS &&
        movingRight) {
        state.SetX(bb.left - static_cast<float>(GameConfig::TILE_SIZE));
        state.SetVelX(0.0f);
        movingRight = false;
    }
}

// Snap left edge to block right when moving left (C# CheckCollisionsLeft).
// C#: if (Left < b.Right && Left > b.Right - size*0.75 && movingLeft)
void CollisionManager::ResolveLeft(PlayerState& state, const AABB& bb,
                                   bool& movingLeft) {
    AABB body = BodyRect(state);
    if (body.left < bb.right &&
        body.left > bb.right - static_cast<float>(GameConfig::TILE_SIZE) *
                                   GameConfig::INTERSECT_STRICTNESS &&
        movingLeft) {
        state.SetX(bb.right);
        state.SetVelX(0.0f);
        movingLeft = false;
    }
}

// ============================================================================
// TriggerBlockHit
// Called from Step 2 (ceiling trigger) when the player's head bumps a block.
// Handles question-mark blocks, coin blocks, brick breaking, audio, and score.
// Extracted into a helper to keep CheckPlayerBlockCollision clean.
// ============================================================================
void CollisionManager::TriggerBlockHit(
    Block& block, PlayerState& state, Camera& camera,
    GameStateManager& gameState, UIManager& uiManager,
    std::vector<Level::SpawnPoint>* outSpawns) {
    if (block.IsHit()) return;  // block already used

    std::string spawnEntity;
    if (block.GetDef().isContainer) {
        spawnEntity = block.GetSpawnContents(state.GetState());
    } else if (block.GetDef().spawner) {
        spawnEntity = block.GetDef().spawnEntity;
    }

    // Convert world-space block centre to PTSD coordinates for floating text
    float blockCX = block.GetWorldX() + GameConfig::TILE_SIZE * 0.5f;
    float blockCY = block.GetWorldY();
    float ptsdX = camera.WorldToScreenX(blockCX) - 640.0f;
    float ptsdY = 360.0f - camera.WorldToScreenY(blockCY);

    if (spawnEntity == "CoinGet") {
        // Coin block: add coins directly, no entity spawn
        gameState.AddCoin();
        gameState.AddScore(200);
        AudioManager::GetInstance().PlaySFX(SFXName::Coin);
        uiManager.AddFloatingText(ptsdX, ptsdY, "+200", 60);
    } else if (!spawnEntity.empty() && outSpawns) {
        Level::SpawnPoint sp;
        sp.entityName = spawnEntity;
        sp.gridX = block.GetGridX();
        sp.gridY = block.GetGridY();
        sp.worldX =
            static_cast<float>(block.GetGridX() * GameConfig::TILE_SIZE);
        sp.worldY =
            static_cast<float>(block.GetGridY() * GameConfig::TILE_SIZE);
        sp.spawned = true;
        outSpawns->push_back(sp);

        if (spawnEntity == "Coin" || spawnEntity == "CoinText") {
            AudioManager::GetInstance().PlaySFX(SFXName::Coin);
        } else {
            AudioManager::GetInstance().PlaySFX(SFXName::Item);
            uiManager.AddFloatingText(ptsdX, ptsdY, "+100", 60);
        }
    }

    block.OnHit(state.GetState());
    // Brick debris is spawned in PlayingSceneHandler via block->JustBroken().
}

// ============================================================================
void CollisionManager::CheckPlayerEntityCollision(
    Player& player, std::vector<std::shared_ptr<Entity>>& entities,
    Camera& camera, GameStateManager& gameState, UIManager& uiManager) {
    if (player.GetState().IsDead()) return;

    PlayerState& ps = player.GetState();
    AABB playerBox = ps.GetHitbox();

    // Reset stomp combo when Mario lands (between stomp chains).
    // Stomp detection requires !ps.IsGrounded(), so the reset happens on the
    // frame Mario touches ground and can never collide with the stomp branch.
    if (ps.IsGrounded()) {
        m_StompCombo = 0;
    }

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
                // Player stomped on enemy from above
                if (es.IsSquishable()) {
                    es.Squish();
                } else if (es.IsKoopaSquash()) {
                    // Koopa Troopa stomped: becomes a shell
                    es.Delete();
                    AudioManager::GetInstance().PlaySFX(SFXName::Squish);
                } else if (entity->GetDef().type == EntityType::BOWSER) {
                    AudioManager::GetInstance().PlaySFX(SFXName::BowserDie);
                } else {
                    AudioManager::GetInstance().PlaySFX(SFXName::Squish);
                }

                // NES-authentic consecutive stomp score multiplier:
                //   1st: base, 2nd: x2, 3rd: x4, 4th: x8, 5th+: cap at 1000
                m_StompCombo++;
                int base = es.GetScoreWorth();
                int scoreWorth;
                if (m_StompCombo >= 5) {
                    scoreWorth = 1000;
                } else {
                    scoreWorth = base * (1 << (m_StompCombo - 1));
                }
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
