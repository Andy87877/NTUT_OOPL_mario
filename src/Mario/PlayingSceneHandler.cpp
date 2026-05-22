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
#include "Mario/Behaviors/ParticleDebris.hpp"
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
    app.GetInputHandler().HandleInput(player->GetState(), app.GetSpeed());

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
                app.GetEntities().push_back(entity);
                app.GetRenderer().AddChild(entity);
            }
        }
    }

    // -- Player state tick --
    ps.Tick();

    // -- Fireball spawn --
    SpawnPlayerFireball(app);

    // -- Update each entity --
    for (auto& entity : app.GetEntities()) {
        if (!entity->GetState().IsActive()) continue;

        auto behavior = entity->GetBehavior();
        if (behavior) {
            behavior->Update(entity->GetState(), *level, *player,
                             app.GetTimer());

            // Process pending spawn request from this entity's behavior
            int spawnType = 0;
            float spawnX = 0.0f, spawnY = 0.0f;
            int spawnDir = 1;
            if (behavior->ConsumeSpawnRequest(spawnType, spawnX, spawnY,
                                              spawnDir)) {
                auto spawnEntityType =
                    static_cast<Mario::EntityType>(spawnType);
                std::string spawnName;
                if (spawnEntityType == Mario::EntityType::FIRE)
                    spawnName = "Fire";
                if (!spawnName.empty()) {
                    Mario::EntityDef def = level->GetEntityDefByName(spawnName);
                    if (def.name.empty()) {
                        def.id = -1;
                        def.name = spawnName;
                        def.type = spawnEntityType;
                        def.isAnimated = true;
                        def.animFrames = 4;
                        def.doesCollide = true;
                        def.isEnemy = false;
                        def.isStatic = false;
                    }
                    auto spawned = Mario::EntityFactory::SpawnEntity(
                        def, spawnX, spawnY, spawnDir, false,
                        app.GetCurrentLevelName());
                    if (spawned) {
                        float speed = 4.0f;
                        spawned->GetState().SetVelX(spawnDir == 1 ? speed
                                                                  : -speed);
                        app.GetEntities().push_back(spawned);
                        app.GetRenderer().AddChild(spawned);
                    }
                }
            }
        }

        // Keep fireball at consistent speed
        if (entity->GetDef().type == Mario::EntityType::FIRE) {
            float spd = std::abs(entity->GetState().GetVelX());
            if (spd < 4.0f) spd = 5.0f;
            entity->GetState().SetVelX(
                entity->GetState().GetDirection() == 1 ? spd : -spd);
        }

        entity->GetState().Tick();

        if (entity->GetState().DoesCollide() &&
            !entity->GetState().IsStatic()) {
            app.GetCollisionManager().CheckEntityBlockCollision(*entity,
                                                                *level);
        }

        entity->UpdateView(app.GetCamera().GetOffset());
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
    app.GetCamera().Update(player->GetWorldX(), level->GetWidthPixels());
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
            app.GetGameState().LoseLife();
            ps.SetDead(true);
        }
    }

    // -- Pit-fall check --
    if (app.GetCollisionManager().CheckPitFall(*player)) {
        app.GetGameState().LoseLife();
        ps.SetDead(true);
        ps.SetControllable(false);
    }

    // -- Death transition --
    if (ps.IsDead()) {
        app.GetDeathTimer() = app.GetTimer() + 80;
        app.TransitionTo(App::State::DEATH);
        Mario::AudioManager::GetInstance().PlayBGM(
            Mario::BGMName::LostALifeTheme);
        LOG_INFO("Player died - entering DEATH state (Lives: {})",
                 app.GetGameState().GetLives());
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
                auto* pb =
                    dynamic_cast<Mario::ParticleDebris*>(debris->GetBehavior());
                if (pb) pb->SetInitialVelocity(p.vx, p.vy);
                app.AddEntityToGame(debris);
            }
        }
    }
}

void PlayingSceneHandler::OnRender(App& app) {
    const std::string& lvl = app.GetCurrentLevelName();
    bool underground = app.GetGameState().IsUnderground() ||
                       lvl.find("u") != std::string::npos || lvl == "1-2" ||
                       lvl == "8-4";
    app.ApplyBackground(underground);
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

    for (const auto& block : app.GetLevel()->GetAllBlocks()) {
        if (!block->IsGoal()) continue;
        if (!playerBox.Intersects(block->GetAABB())) continue;

        LOG_INFO("Flagpole reached at block ({}, {})", block->GetGridX(),
                 block->GetGridY());
        Mario::AudioManager::GetInstance().PlaySFX(Mario::SFXName::Flagpole);
        Mario::AudioManager::GetInstance().PlayBGM(
            lvl == "8-4" ? Mario::BGMName::CastleCompleteTheme
                         : Mario::BGMName::LevelCompleteTheme);
        app.GetLevelCompleteCtrl().StartFlagpole(*player, app.GetFlagEntity(),
                                                 block.get());
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
    for (const auto& block : app.GetLevel()->GetAllBlocks()) {
        if (!block->IsGoal()) continue;
        Mario::AABB bBox = block->GetAABB();
        if (playerBox.right > bBox.left && playerBox.left < bBox.right) {
            if (block->GetWorldY() < topmostY) {
                topmostY = block->GetWorldY();
                topmostGoal = block.get();
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

    // Full-body AABB (C# GetRecPosition), expanded 1px downward so a player
    // standing exactly on a block top edge still registers as intersecting.
    const float TS = static_cast<float>(Mario::GameConfig::TILE_SIZE);
    Mario::AABB playerBox = Mario::AABB::FromPosSize(
        ps.GetX(), ps.GetY(),
        TS, static_cast<float>(ps.GetHeight()) + 1.0f);

    bool pipeDown1 = false, pipeDown2 = false;
    bool pipeRight1 = false, pipeRight2 = false;
    float pipeDX = 0.0f, pipeDY = 0.0f;
    float pipeRX = 0.0f, pipeRY = 0.0f;

    for (const auto& block : app.GetLevel()->GetAllBlocks()) {
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
    if (!player || player->GetState().IsDead() ||
        player->GetState().IsInvincible())
        return;
    app.GetCollisionManager().CheckPlayerEntityCollision(
        *player, app.GetEntities(), app.GetCamera(), app.GetGameState(),
        app.GetUIManager());
}

void PlayingSceneHandler::CheckEntityEntityCollision(App& app) const {
    app.GetCollisionManager().CheckEntityEntityCollision(app.GetEntities(),
                                                         app.GetGameState());
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
