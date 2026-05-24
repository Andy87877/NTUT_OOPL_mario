/**
 * @file PlayingSceneHandler.cpp
 * @brief Main gameplay scene handler implementation.
 *        Logic moved from App::UpdatePlaying() and factored into helpers.
 * @inheritance ISceneHandler <- PlayingSceneHandler
 */
#include "Mario/PlayingSceneHandler.hpp"

#include <cmath>
#include <limits>

#include "App.hpp"
#include "Mario/AudioManager.hpp"
#include "Mario/EntityFactory.hpp"
#include "Mario/GameConfig.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include "Util/Logger.hpp"

// OpenGL clear color for ApplyBackground calls
#include <GL/glew.h>

namespace Mario {

// ============================================================================
// Update — primary gameplay loop
// ============================================================================
void PlayingSceneHandler::Update(App& app) {
    // ESC -> pause
    if (Util::Input::IsKeyDown(Util::Keycode::ESCAPE)) {
        app.GetESCMenuSelection() = 0;
        app.TransitionTo(App::State::ESC_MENU);
        Mario::AudioManager::GetInstance().PlaySFX(Mario::SFXName::Pause);
        Mario::AudioManager::GetInstance().StopBGM();
        LOG_INFO("Game paused - entering ESC_MENU state");
        return;
    }

    auto& player = app.GetPlayer();
    auto& level = app.GetLevel();
    if (!player || !level) return;

    // -- Input (MVC Controller layer) --
    app.GetInputHandler().HandleInput(player->GetState(), app.GetSpeed(),
                                      *level);

    // -- Step moving platforms and carry player --
    // Must run before gravity so the player is repositioned before physics.
    auto& ps = player->GetState();
    for (auto* plat : level->GetMovingPlatforms()) {
        AABB prevAABB = plat->GetAABB();
        plat->StepMovement();

        if (ps.IsGrounded()) {
            AABB playerBox = ps.GetHitbox();
            float gap = std::abs(playerBox.bottom - prevAABB.top);
            bool xOverlap = (playerBox.left < prevAABB.right &&
                             playerBox.right > prevAABB.left);
            if (gap < 2.0f && xOverlap) {
                ps.SetX(ps.GetX() + plat->GetLastDeltaX());
                ps.SetY(ps.GetY() + plat->GetLastDeltaY());
            }
        }
    }

    // -- Apply gravity + position --
    float yDelta = ps.ApplyGravity();
    ps.SetX(ps.GetX() + ps.GetVelX());
    ps.SetY(ps.GetY() + yDelta);

    // -- Player-block collision --
    std::vector<Mario::Level::SpawnPoint> newSpawns;
    app.GetCollisionManager().CheckPlayerBlockCollision(
        *player, *level, app.GetCamera(), app.GetGameState(),
        app.GetUIManager(), &newSpawns);

    // -- Spawn entities from hit blocks (coin-blocks, etc.) --
    if (!newSpawns.empty()) {
        for (auto& sp : newSpawns) {
            auto entity = Mario::EntityFactory::SpawnEntity(
                level->GetEntityDefByName(sp.entityName), sp.worldX, sp.worldY,
                1, true);
            if (entity) {
                app.AddEntityToGame(entity);
            }
        }
    }

    // -- Player state tick --
    ps.Tick();

    // -- Fireball spawn --
    SpawnPlayerFireball(app);

    // -- Update each entity --
    std::vector<std::shared_ptr<Entity>> newEntities;
    float cameraOffset = app.GetCamera().GetOffset();
    // Activation window for updating entities: 200px buffer on left/right of
    // viewport. This matches the C# viewport-based entity activation.
    float leftBound = cameraOffset - 200.0f;
    float rightBound = cameraOffset + GameConfig::WINDOW_WIDTH + 200.0f;

    // Single-pass: detect in-viewport Bowser AND silence the fire spawner in
    // the same loop, avoiding the previous two separate full-entity traversals.
    bool bowserInViewport = false;
    for (auto& entity : app.GetEntities()) {
        if (!entity) continue;
        if (entity->GetDef().type == EntityType::BOWSER &&
            entity->GetState().IsActive()) {
            float ex = entity->GetWorldX();
            float ew = static_cast<float>(entity->GetState().GetWidth());
            if (ex + ew >= leftBound && ex <= rightBound) {
                bowserInViewport = true;
                break;
            }
        }
    }
    if (bowserInViewport) {
        for (auto& entity : app.GetEntities()) {
            if (entity &&
                entity->GetDef().type == EntityType::CASTLE_FIRE_SPAWNER) {
                entity->GetState().Delete();
            }
        }
    }

    for (auto& entity : app.GetEntities()) {
        if (!entity->GetState().IsActive()) continue;

        float entityX = entity->GetWorldX();
        float entityWidth = static_cast<float>(entity->GetState().GetWidth());
        bool inUpdateWindow =
            (entityX + entityWidth >= leftBound && entityX <= rightBound);

        // Always update short-lived particles/debris, and only update regular
        // entities when within viewport window. Also fireballs shot by the
        // player should keep moving even if they go slightly off screen.
        auto behavior = entity->GetBehavior();
        bool isParticle = behavior && behavior->AlwaysUpdate();
        bool isPlayerFireball = (entity->GetDef().type == EntityType::FIRE &&
                                 !entity->GetState().IsEnemy());

        if (inUpdateWindow || isParticle || isPlayerFireball) {
            if (behavior) {
                behavior->Update(entity->GetState(), *level, *player,
                                 app.GetTimer());

                // Process pending spawn requests from this entity's behavior.
                // EntityFactory::MakeProjectileDef owns all EntityDef
                // construction — no inline config here.
                EntityType spawnType = EntityType::UNKNOWN;
                float spawnX = 0.0f, spawnY = 0.0f;
                int spawnDir = 1;
                while (behavior->ConsumeSpawnRequest(spawnType, spawnX, spawnY,
                                                     spawnDir)) {
                    bool isEnemyProjectile =
                        entity->GetDef().type == Mario::EntityType::BOWSER ||
                        entity->GetDef().type == Mario::EntityType::AXE_KOOPA ||
                        entity->GetDef().type ==
                            Mario::EntityType::CASTLE_FIRE_SPAWNER;

                    Mario::EntityDef def =
                        Mario::EntityFactory::MakeProjectileDef(
                            spawnType, isEnemyProjectile, *level);
                    if (def.name.empty()) continue;

                    auto spawned = Mario::EntityFactory::SpawnEntity(
                        def, spawnX, spawnY, spawnDir, false,
                        app.GetCurrentLevelName());
                    if (!spawned) continue;

                    if (spawnType == Mario::EntityType::FIRE) {
                        float speed = isEnemyProjectile ? 3.0f : 4.0f;
                        if (isEnemyProjectile) {
                            spawned->GetState().SetGravity(false);
                        }
                        spawned->GetState().SetVelX(spawnDir == 1 ? speed
                                                                  : -speed);
                    } else if (spawnType == Mario::EntityType::AXE_PROJECTILE) {
                        // Parabolic arc targeted at Mario's current position
                        float dx = std::abs(player->GetWorldX() - spawnX);
                        float launchVelY = std::max(
                            8.0f, std::min(15.0f, 8.0f + (dx / 100.0f) * 1.5f));
                        float flightTime = launchVelY * 7.5f;
                        float throwSpeed =
                            std::max(1.5f, std::min(6.5f, dx / flightTime));
                        spawned->GetState().SetVelX(
                            spawnDir == 1 ? throwSpeed : -throwSpeed);
                        spawned->GetState().SetFallHeight(launchVelY);
                    }

                    newEntities.push_back(spawned);
                }
            }

            // Fireball speed enforcement is now handled inside
            // FireballBehavior::Update — no clamp needed here.

            entity->GetState().Tick();

            if (entity->GetState().DoesCollide() &&
                !entity->GetState().IsStatic()) {
                app.GetCollisionManager().CheckEntityBlockCollision(
                    *entity, *level, &newEntities);
            }
        }

        entity->UpdateView(app.GetCamera().GetOffset());
    }

    if (!newEntities.empty()) {
        for (auto& entity : newEntities) {
            app.AddEntityToGame(entity);
        }
    }

    // -- Collision checks --
    CheckPlayerEntityCollision(app);
    CheckEntityEntityCollision(app);

    if (app.GetCurrentLevelName() == "8-4") {
        CheckAxeCollision(app);
    }

    CheckFlagpoleCollision(app);
    CheckPipeCollision(app);

    // -- Camera + level blocks --
    app.GetCamera().Update(player->GetWorldX(), level->GetWidthPixels(),
                           app.GetCurrentLevelName(),
                           app.GetLevelCompleteCtrl().IsActive());
    level->UpdateBlocks(app.GetCamera().GetOffset());

    // -- Brick-debris particles --
    SpawnBrickDebris(app);

    // -- Player view --
    player->UpdateView(app.GetCamera().GetOffset());

    // -- Game timer --
    if (ps.IsControllable()) {
        int oldTime = app.GetGameState().GetTimeRemaining();
        app.GetGameState().Tick();
        int newTime = app.GetGameState().GetTimeRemaining();
        if (oldTime > 100 && newTime <= 100) {
            app.PlayCurrentBGM();  // switch to hurry-up theme
        }
        if (app.GetGameState().IsTimeUp()) {
            ps.StartDeathAnimation();
        }
    }

    // -- Pit-fall check --
    if (app.GetCollisionManager().CheckPitFall(*player)) {
        ps.StartDeathAnimation();
    }

    // -- Death transition --
    if (ps.IsDead()) {
        app.TransitionTo(App::State::DEATH);
    }

    // -- Cleanup dead entities --
    CleanupDeadEntities(app);
}

// ============================================================================
// SpawnPlayerFireball
// ============================================================================
void PlayingSceneHandler::SpawnPlayerFireball(App& app) const {
    auto& player = app.GetPlayer();
    auto& ps = player->GetState();

    if (!ps.IsFireShooting() || ps.GetSpecialCounter() != 1) return;

    int dir = ps.IsFacingRight() ? 1 : -1;
    float fbX = player->GetWorldX() + (dir == 1 ? ps.GetWidth() / 2.0f : 0.0f);
    float fbY = player->GetWorldY() + ps.GetHeight() / 4.0f;

    Mario::EntityDef def = app.GetLevel()->GetEntityDefByName("Fire");
    if (def.name.empty()) {
        def.id = -1;
        def.name = "Fire";
        def.type = Mario::EntityType::FIRE;
        def.isAnimated = true;
        def.animFrames = 4;
        def.doesCollide = true;
        def.isEnemy = false;
        def.isStatic = false;
    }

    auto fb = Mario::EntityFactory::SpawnEntity(
        def, fbX, fbY, dir == 1 ? 1 : 0, false, app.GetCurrentLevelName());
    if (fb) {
        fb->GetState().SetVelX(dir * 5.0f);
        app.GetEntities().push_back(fb);
        app.GetRenderer().AddChild(fb);
        Mario::AudioManager::GetInstance().PlaySFX(Mario::SFXName::FireBall);
    }
}

// ============================================================================
// SpawnBrickDebris — four particles for every block that just broke
// ============================================================================
void PlayingSceneHandler::SpawnBrickDebris(App& app) const {
    auto& level = app.GetLevel();
    auto& camera = app.GetCamera();

    auto blocks = level->GetBlocksInRange(
        camera.GetOffset() - 100.0f,
        camera.GetOffset() + Mario::GameConfig::WINDOW_WIDTH + 100.0f);

    for (auto* block : blocks) {
        if (!block || !block->JustBroken()) continue;

        float bx = block->GetWorldX();
        float by = block->GetWorldY();
        float bs = static_cast<float>(Mario::GameConfig::TILE_SIZE);

        // C# reference: debris entity name = blockName + "Break"
        std::string debrisName = block->GetName() + "Break";
        const Mario::EntityDef& def = level->GetEntityDefByName(debrisName);
        if (def.id == -1) continue;  // No debris definition for this block type

        struct Piece {
            float dx, dy, vx, vy;
        };
        static constexpr Piece kPieces[4] = {
            {-0.25f, -0.25f, -3.0f, -6.0f},  // top-left
            {0.25f, -0.25f, 3.0f, -6.0f},    // top-right
            {-0.25f, 0.25f, -3.0f, -4.0f},   // bottom-left
            {0.25f, 0.25f, 3.0f, -4.0f},     // bottom-right
        };

        for (const auto& p : kPieces) {
            auto debris = Mario::EntityFactory::SpawnEntity(
                def, bx + p.dx * bs, by + p.dy * bs, 0, false,
                app.GetCurrentLevelName());
            if (debris) {
                debris->GetBehavior()->OnSpawned(p.vx, p.vy);
                app.AddEntityToGame(debris);
            }
        }
    }
}

void PlayingSceneHandler::OnRender(App& app) {
    app.ApplyBackground();
    app.GetRenderer().Update();
    app.GetUIManager().Update(Mario::UIManager::State::PLAYING);
}

// ============================================================================
// CheckFlagpoleCollision — owned by PlayingSceneHandler (PLAYING state only)
// ============================================================================
void PlayingSceneHandler::CheckFlagpoleCollision(App& app) const {
    auto& player = app.GetPlayer();
    if (!player || player->GetState().IsDead()) return;
    if (app.GetLevelCompleteCtrl().IsActive()) return;

    Mario::PlayerState& ps = player->GetState();
    Mario::AABB playerBox = ps.GetHitbox();
    const std::string& lvl = app.GetCurrentLevelName();

    for (const auto& block : app.GetLevel()->GetGoalBlocks()) {
        if (!playerBox.Intersects(block->GetAABB())) continue;

        LOG_INFO("Flagpole reached at block ({}, {})", block->GetGridX(),
                 block->GetGridY());
        Mario::AudioManager::GetInstance().PlaySFX(Mario::SFXName::Flagpole);
        Mario::AudioManager::GetInstance().PlayBGM(
            lvl == "8-4" ? Mario::BGMName::CastleCompleteTheme
                         : Mario::BGMName::LevelCompleteTheme);
        app.GetLevelCompleteCtrl().StartFlagpole(*player, app.GetFlagEntity(),
                                                 block);
        app.GetGameState().StopTime();
        app.TransitionTo(App::State::FLAGPOLE);
        return;
    }

    // Fallback: if Mario is horizontally within the pole column but above the
    // topmost goal block (jumped high), trigger goal using the topmost goal
    // block. This prevents Mario from jumping over the flagpole.
    // C# makes this physically impossible via jump-height caps; we add an
    // explicit X-range check as a safety net.
    const Block* topmostGoal = nullptr;
    float topmostY = std::numeric_limits<float>::max();
    for (const auto& block : app.GetLevel()->GetGoalBlocks()) {
        Mario::AABB bBox = block->GetAABB();
        if (playerBox.right > bBox.left && playerBox.left < bBox.right) {
            if (block->GetWorldY() < topmostY) {
                topmostY = block->GetWorldY();
                topmostGoal = block;
            }
        }
    }
    if (topmostGoal) {
        LOG_INFO("Flagpole X-range fallback at block ({}, {})",
                 topmostGoal->GetGridX(), topmostGoal->GetGridY());
        Mario::AudioManager::GetInstance().PlaySFX(Mario::SFXName::Flagpole);
        Mario::AudioManager::GetInstance().PlayBGM(
            lvl == "8-4" ? Mario::BGMName::CastleCompleteTheme
                         : Mario::BGMName::LevelCompleteTheme);
        app.GetLevelCompleteCtrl().StartFlagpole(*player, app.GetFlagEntity(),
                                                 topmostGoal);
        app.GetGameState().StopTime();
        app.TransitionTo(App::State::FLAGPOLE);
    }
}

// ============================================================================
// CheckPipeCollision — owned by PlayingSceneHandler (PLAYING state only)
// Ported from C# Form1.cs pipe detection logic (pipeCheck1/pipeCheck2 block).
//
// Root cause of previous bug: AABB::Intersects uses strict (bottom > top).
// After CollisionManager::ResolveDown snaps Mario to exactly block.top,
// Mario's bottom == pipe.top, so Intersects returns false and the pipe
// is never detected.
// Fix: use a full-body rect (C# GetRecPosition) expanded +1px downward so
// standing on a block's top edge still counts as an intersection.
//
// Centering condition (down pipe) ported from C#:
//   pipeRec.X < Mario.X < pipeRec.X + scaleSize/1.25
//   Scaled to TILE_SIZE=45: pipeDX < ps.GetX() < pipeDX + TILE_SIZE/1.25
//
// Vertical alignment (right pipe) ported from C#:
//   pipeRec.Y + scaleSize + 1 > Mario.Y
//   Scaled to TILE_SIZE=45: pipeRY + TILE_SIZE + 1 > ps.GetY()
// ============================================================================
void PlayingSceneHandler::CheckPipeCollision(App& app) const {
    auto& player = app.GetPlayer();
    if (!player || player->GetState().IsDead()) return;
    if (app.GetLevelCompleteCtrl().IsActive()) return;

    Mario::PlayerState& ps = player->GetState();

    // Full-body AABB (C# GetRecPosition), expanded 2px on all sides so that
    // even after CollisionManager snaps Mario out of blocks in PHASE 4,
    // he still registers as intersecting adjacent pipe blocks.
    const float TS = static_cast<float>(Mario::GameConfig::TILE_SIZE);
    Mario::AABB playerBox =
        Mario::AABB::FromPosSize(ps.GetX() - 2.0f, ps.GetY() - 2.0f, TS + 4.0f,
                                 static_cast<float>(ps.GetHeight()) + 4.0f);

    bool pipeDown1 = false, pipeDown2 = false;
    bool pipeRight1 = false, pipeRight2 = false;
    float pipeDX = 0.0f, pipeDY = 0.0f;
    float pipeRX = 0.0f, pipeRY = 0.0f;

    // Only search blocks in the player's immediate vicinity (±1 tile).
    // Avoids scanning the entire level (3520 blocks) for just 2–4 pipe tiles.
    std::vector<Block*> nearBlocks;
    app.GetLevel()->QueryBlocksInRange(playerBox.left - TS,
                                       playerBox.right + TS, nearBlocks);
    for (const auto* block : nearBlocks) {
        Mario::AABB bBox = block->GetAABB();
        if (!playerBox.Intersects(bBox)) continue;
        int id = block->GetBlockID();
        if (id == Mario::GameConfig::PIPE_DOWN_LEFT) {
            pipeDown1 = true;
            pipeDX = block->GetWorldX();
            pipeDY = block->GetWorldY();
        }
        if (id == Mario::GameConfig::PIPE_DOWN_RIGHT) pipeDown2 = true;
        if (id == Mario::GameConfig::PIPE_RIGHT_TOP) {
            pipeRight1 = true;
            pipeRX = block->GetWorldX();
            pipeRY = block->GetWorldY();
        }
        if (id == Mario::GameConfig::PIPE_RIGHT_BOT) {
            pipeRight2 = true;
            if (!pipeRight1) {
                pipeRX = block->GetWorldX();
                pipeRY = block->GetWorldY();
            }
        }
    }

    // Down pipe: both halves must intersect + Mario grounded + Down key held.
    // Centering check (C#): pipeDX < Mario.X < pipeDX + TILE_SIZE/1.25
    if (pipeDown1 && pipeDown2 && ps.IsGrounded() &&
        Util::Input::IsKeyPressed(Util::Keycode::DOWN)) {
        float maxOffset = TS / 1.25f;  // = 36px at TILE_SIZE=45
        if (ps.GetX() > pipeDX && ps.GetX() < pipeDX + maxOffset) {
            LOG_INFO("Entering pipe DOWN at ({}, {})", pipeDX, pipeDY);
            app.GetLevelCompleteCtrl().StartPipeWarp(*player, "Down", pipeDX,
                                                     pipeDY);
            Mario::AudioManager::GetInstance().PlaySFX(Mario::SFXName::Warp);
            int time = app.GetGameState().GetTimeRemaining();
            Mario::AudioManager::GetInstance().PlayBGM(
                (time <= 100 && time > 0) ? Mario::BGMName::IntoThePipeHurryUp
                                          : Mario::BGMName::IntoThePipeTheme);
            app.GetGameState().StopTime();
            app.TransitionTo(App::State::PIPE_WARP);
            return;
        }
    }

    // Right pipe: any half must intersect + grounded + Right key held.
    // Vertical alignment (C#): pipeRY + TILE_SIZE + 1 > Mario.Y
    if ((pipeRight1 || pipeRight2) && ps.IsGrounded() &&
        Util::Input::IsKeyPressed(Util::Keycode::RIGHT)) {
        if (pipeRY + TS + 1.0f > ps.GetY()) {
            LOG_INFO("Entering pipe RIGHT at ({}, {})", pipeRX, pipeRY);
            app.GetLevelCompleteCtrl().StartPipeWarp(*player, "Right", pipeRX,
                                                     pipeRY);
            Mario::AudioManager::GetInstance().PlaySFX(Mario::SFXName::Warp);
            int time = app.GetGameState().GetTimeRemaining();
            Mario::AudioManager::GetInstance().PlayBGM(
                (time <= 100 && time > 0) ? Mario::BGMName::IntoThePipeHurryUp
                                          : Mario::BGMName::IntoThePipeTheme);
            app.GetGameState().StopTime();
            if (app.GetGameState().GetLevelName() == "1-2") {
                app.GetGameState().SetNextLevelOverride("8-4");
            }
            app.TransitionTo(App::State::PIPE_WARP);
            return;
        }
    }
}

// ============================================================================
// CheckAxeCollision — owned by PlayingSceneHandler (8-4 only)
// ============================================================================
void PlayingSceneHandler::CheckAxeCollision(App& app) const {
    auto& player = app.GetPlayer();
    if (!player || player->GetState().IsDead()) return;
    if (app.GetLevelCompleteCtrl().IsActive()) return;

    Mario::AABB playerBox = player->GetState().GetHitbox();
    for (const auto& entity : app.GetEntities()) {
        if (entity->GetState().GetName() != "Axe") continue;
        float axeX = entity->GetState().GetX();
        float axeY = entity->GetState().GetY();
        Mario::AABB extendedAxeBox{axeX - 5.0f, axeY, axeX + 55.0f,
                                   axeY + 120.0f};
        if (playerBox.Intersects(extendedAxeBox)) {
            // In 8-4 there are fake boss rooms with axes that should collapse
            // the bridge but NOT end the game.  Only the final real Axe (past
            // the last pipe warp, at ~col 342 = worldX 15390) ends the game.
            // We detect the real axe by checking for a Princess entity — the
            // Princess is only spawned in the final boss room.
            bool hasPrincess = false;
            for (const auto& e : app.GetEntities()) {
                if (e->GetState().GetName() == "Princess" &&
                    e->GetState().IsActive()) {
                    hasPrincess = true;
                    break;
                }
            }

            entity->GetState().SetActive(false);
            app.GetGameState().StopTime();
            Mario::AudioManager::GetInstance().StopBGM();
            Mario::AudioManager::GetInstance().PlaySFX(Mario::SFXName::Break);

            if (hasPrincess) {
                LOG_INFO(
                    "Axe touched (real boss room)! Starting 8-4 ending "
                    "sequence.");
                app.TransitionTo(App::State::AXE_SEQUENCE);
            } else {
                LOG_INFO(
                    "Axe touched (fake boss room) - bridge collapses, game "
                    "continues.");
                // Resume time and BGM so Mario can continue
                app.GetGameState().StartTime();
                app.PlayCurrentBGM();
            }
            return;
        }
    }
}

// ============================================================================
// CheckPlayerEntityCollision / CheckEntityEntityCollision / CleanupDeadEntities
// ============================================================================
void PlayingSceneHandler::CheckPlayerEntityCollision(App& app) const {
    auto& player = app.GetPlayer();
    if (!player || player->GetState().IsDead()) return;
    app.GetCollisionManager().CheckPlayerEntityCollision(
        *player, app.GetEntities(), app.GetCamera(), app.GetGameState(),
        app.GetUIManager());
}

void PlayingSceneHandler::CheckEntityEntityCollision(App& app) const {
    app.GetCollisionManager().CheckEntityEntityCollision(
        app.GetEntities(), app.GetGameState(), app.GetCamera().GetOffset());
}

void PlayingSceneHandler::CleanupDeadEntities(App& app) const {
    auto& entities = app.GetEntities();
    entities.erase(std::remove_if(entities.begin(), entities.end(),
                                  [&](const std::shared_ptr<Mario::Entity>& e) {
                                      if (!e->GetState().IsActive()) {
                                          app.GetRenderer().RemoveChild(e);
                                          return true;
                                      }
                                      return false;
                                  }),
                   entities.end());
}

}  // namespace Mario
