/**
 * @file PlayingSceneHandler.cpp
 * @brief Main gameplay scene handler implementation.
 *        Logic moved from App::UpdatePlaying() and factored into helpers.
 * @inheritance ISceneHandler <- PlayingSceneHandler
 */
#include "Mario/Scenes/PlayingSceneHandler.hpp"

#include <cmath>
#include <limits>

#include "App.hpp"
#include "Mario/Collision/BlockContactResolver.hpp"
#include "Mario/Core/GameConfig.hpp"
#include "Mario/Level/EntityFactory.hpp"
#include "Mario/Scenes/AxeSequenceSceneHandler.hpp"
#include "Mario/Scenes/FlagpoleSceneHandler.hpp"
#include "Mario/Scenes/PipeWarpSceneHandler.hpp"
#include "Mario/Services/AudioManager.hpp"
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

    // -- Step moving platforms and carry player (OOP virtual dispatch) --
    // Must run before gravity so the player is repositioned before physics.
    auto& ps = player->GetState();
    ps.SetOnMovingPlatform(false);  // Reset per-frame platform-carry flag
    bool onStaticBlock =
        BlockContactResolver::IsPlayerOnStaticBlock(ps, *level);
    for (auto* plat : level->GetMovingPlatforms()) {
        AABB prevAABB = plat->GetAABB();
        plat->StepMovement();
        plat->TryCarryPlayer(ps, prevAABB, onStaticBlock);
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

    // -- Star BGM transition check --
    bool isStar = ps.IsStar();
    if (isStar != m_WasStarActive) {
        app.PlayCurrentBGM();
        m_WasStarActive = isStar;
    }

    // -- Fireball spawn --
    SpawnPlayerFireball(app);

    // -- Update each entity --
    std::vector<std::shared_ptr<Entity>> newEntities;
    float cameraOffset = app.GetCamera().GetOffset();
    // Activation window for updating entities: 200px buffer on left/right of
    // viewport. This matches the C# viewport-based entity activation.
    float leftBound = cameraOffset - 200.0f;
    float rightBound = cameraOffset + GameConfig::WINDOW_WIDTH + 200.0f;

    for (auto& entity : app.GetEntities()) {
        if (!entity) continue;
        if (!entity->GetState().IsActive()) continue;

        float entityX = entity->GetWorldX();
        float entityWidth = static_cast<float>(entity->GetState().GetWidth());
        bool inUpdateWindow =
            (entityX + entityWidth >= leftBound && entityX <= rightBound);

        // Always update short-lived particles/debris, and only update regular
        // entities when within viewport window.
        // IsPlayerFireball() is resolved polymorphically via IEntityBehavior —
        // no EntityType or IsEnemy() check here (OCP / Strategy Pattern).
        auto behavior = entity->GetBehavior();
        bool isParticle = behavior && behavior->AlwaysUpdate();
        bool isPlayerFireball = behavior && behavior->IsPlayerFireball();

        if (inUpdateWindow || isParticle || isPlayerFireball) {
            if (behavior) {
                behavior->Update(entity->GetState(), *level, *player,
                                 app.GetTimer());

                // Process pending spawn requests from this entity's behavior.
                EntityType spawnType = EntityType::UNKNOWN;
                float spawnX = 0.0f, spawnY = 0.0f;
                int spawnDir = 1;
                while (behavior->ConsumeSpawnRequest(spawnType, spawnX, spawnY,
                                                     spawnDir)) {
                    auto spawned = EntityFactory::SpawnProjectile(
                        entity, spawnType, spawnX, spawnY, spawnDir, *player,
                        *level, app.GetCurrentLevelName());
                    if (spawned) {
                        newEntities.push_back(spawned);
                    }
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

    // IsBossLevel() encapsulates the "8-4" name inside Level (OCP/DRY).
    if (level->IsBossLevel()) {
        CheckAxeCollision(app);
    }

    CheckFlagpoleCollision(app);
    CheckPipeCollision(app);

    // -- Camera + level blocks --
    app.GetCamera().Update(player->GetWorldX(), level->GetWidthPixels(),
                           app.GetCurrentLevelName(), false);
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

    // Delegate all EntityDef construction and velocity assignment to the
    // factory. PlayingSceneHandler only computes the spawn position from player
    // state.
    auto fb = Mario::EntityFactory::SpawnFromPlayer(
        *player, Mario::EntityType::FIRE, fbX, fbY, dir == 1 ? 1 : 0,
        *app.GetLevel(), app.GetCurrentLevelName());

    if (fb) {
        app.AddEntityToGame(fb);
        Mario::AudioManager::GetInstance().PlaySFX(Mario::SFXName::FireBall);
    }
}

// ============================================================================
// SpawnBrickDebris — four particles for every block that just broke
// ============================================================================
void PlayingSceneHandler::SpawnBrickDebris(App& app) const {
    auto& level = app.GetLevel();
    auto& camera = app.GetCamera();
    thread_local std::vector<Block*> debrisQueryBuffer;
    level->QueryBlocksInRange(
        camera.GetOffset() - 100.0f,
        camera.GetOffset() + Mario::GameConfig::WINDOW_WIDTH + 100.0f,
        debrisQueryBuffer);

    for (auto* block : debrisQueryBuffer) {
        if (!block || !block->JustBroken()) continue;

        float bx = block->GetWorldX();
        float by = block->GetWorldY();
        float bs = static_cast<float>(Mario::GameConfig::TILE_SIZE);

        std::string debrisName = block->GetDebrisEntityName();
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
// TriggerFlagpoleEntry — shared helper (DRY)
// Both the AABB-intersection path and the X-range fallback path in
// CheckFlagpoleCollision need exactly the same state changes.
// Extracting here removes ~12 lines of duplication and makes the BGM
// selection (IsBossLevel) centralised in one place.
// ============================================================================
void PlayingSceneHandler::TriggerFlagpoleEntry(App& app, PlayerState& ps,
                                               float poleX,
                                               const Level& level) const {
    Mario::AudioManager::GetInstance().PlaySFX(Mario::SFXName::Flagpole);
    Mario::AudioManager::GetInstance().PlayBGM(
        level.IsBossLevel() ? Mario::BGMName::CastleCompleteTheme
                            : Mario::BGMName::LevelCompleteTheme);
    app.GetGameState().StopTime();
    app.TransitionTo(App::State::FLAGPOLE);

    // Align Mario to the pole and stop movement.
    ps.SetControllable(false);
    ps.SetPoleSliding(true);
    ps.SetX(poleX);
    ps.SetVelX(0.0f);
    ps.SetVelY(0.0f);

    // Cancel star power — the level-complete theme replaces the star theme.
    if (ps.GetPowerState() == PowerState::SMALL_STAR ||
        ps.GetPowerState() == PowerState::BIG_STAR) {
        ps.SetPowerState(ps.GetPowerState() == PowerState::BIG_STAR
                             ? PowerState::BIG
                             : PowerState::SMALL);
    }
}

// ============================================================================
// CheckFlagpoleCollision — owned by PlayingSceneHandler (PLAYING state only)
// ============================================================================
void PlayingSceneHandler::CheckFlagpoleCollision(App& app) const {
    auto& player = app.GetPlayer();
    if (!player || player->GetState().IsDead()) return;

    Mario::PlayerState& ps = player->GetState();
    Mario::AABB playerBox = ps.GetHitbox();
    const auto& level = app.GetLevel();

    for (const auto& block : level->GetGoalBlocks()) {
        if (!playerBox.Intersects(block->GetAABB())) continue;
        LOG_INFO("Flagpole reached at block ({}, {})", block->GetGridX(),
                 block->GetGridY());
        float poleX = block->GetWorldX() - GameConfig::TILE_SIZE * 0.4f;
        TriggerFlagpoleEntry(app, ps, poleX, *level);
        return;
    }

    // Fallback: if Mario is horizontally within the pole column but above the
    // topmost goal block (jumped high), trigger goal using the topmost goal
    // block. This prevents Mario from jumping over the flagpole.
    // C# makes this physically impossible via jump-height caps; we add an
    // explicit X-range check as a safety net.
    const Block* topmostGoal = nullptr;
    float topmostY = std::numeric_limits<float>::max();
    for (const auto& block : level->GetGoalBlocks()) {
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
        float poleX = topmostGoal->GetWorldX() - GameConfig::TILE_SIZE * 0.4f;
        TriggerFlagpoleEntry(app, ps, poleX, *level);
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
        (Util::Input::IsKeyPressed(Util::Keycode::DOWN) ||
         Util::Input::IsKeyPressed(Util::Keycode::S))) {
        float maxOffset = TS / 1.25f;  // = 36px at TILE_SIZE=45
        if (ps.GetX() > pipeDX && ps.GetX() < pipeDX + maxOffset) {
            LOG_INFO("Entering pipe DOWN at ({}, {})", pipeDX, pipeDY);
            Mario::AudioManager::GetInstance().PlaySFX(Mario::SFXName::Warp);
            int time = app.GetGameState().GetTimeRemaining();
            Mario::AudioManager::GetInstance().PlayBGM(
                (time <= 100 && time > 0) ? Mario::BGMName::IntoThePipeHurryUp
                                          : Mario::BGMName::IntoThePipeTheme);
            app.GetGameState().StopTime();
            // Level knows its own sublevel name — no "1-2" hardcode here.
            if (app.GetLevel()->HasSubLevel()) {
                app.GetGameState().SetNextLevelOverride(
                    app.GetLevel()->GetSubLevelName());
            }
            app.GetGameState().SetWarpInfo("Down", pipeDX, pipeDY);
            app.TransitionTo(App::State::PIPE_WARP);
            return;
        }
    }

    // Right pipe: any half must intersect + grounded + Right key held.
    // Vertical alignment (C#): pipeRY + TILE_SIZE + 1 > Mario.Y
    if ((pipeRight1 || pipeRight2) && ps.IsGrounded() &&
        (Util::Input::IsKeyPressed(Util::Keycode::RIGHT) ||
         Util::Input::IsKeyPressed(Util::Keycode::D))) {
        if (pipeRY + TS + 1.0f > ps.GetY()) {
            LOG_INFO("Entering pipe RIGHT at ({}, {})", pipeRX, pipeRY);
            Mario::AudioManager::GetInstance().PlaySFX(Mario::SFXName::Warp);
            int time = app.GetGameState().GetTimeRemaining();
            Mario::AudioManager::GetInstance().PlayBGM(
                (time <= 100 && time > 0) ? Mario::BGMName::IntoThePipeHurryUp
                                          : Mario::BGMName::IntoThePipeTheme);
            app.GetGameState().StopTime();
            // Level knows its own sublevel name — no "1-2" hardcode here.
            if (app.GetLevel()->HasSubLevel()) {
                app.GetGameState().SetNextLevelOverride(
                    app.GetLevel()->GetSubLevelName());
            }
            app.GetGameState().SetWarpInfo("Right", pipeRX, pipeRY);
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

    Mario::AABB playerBox = player->GetState().GetHitbox();
    for (const auto& entity : app.GetEntities()) {
        if (!entity || !entity->GetBehavior() ||
            !entity->GetBehavior()->IsAxe())
            continue;
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
                if (e && e->GetBehavior() && e->GetBehavior()->IsPrincess() &&
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
                // Collapse the bridge blocks right now!
                auto& level = app.GetLevel();
                if (level) {
                    level->CollapseBridge();
                }
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
