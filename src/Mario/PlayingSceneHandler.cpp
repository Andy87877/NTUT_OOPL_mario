/**
 * @file PlayingSceneHandler.cpp
 * @brief Main gameplay scene handler implementation.
 *        Logic moved from App::UpdatePlaying() and factored into helpers.
 * @inheritance ISceneHandler <- PlayingSceneHandler
 */
#include "Mario/PlayingSceneHandler.hpp"

#include <cmath>

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
    app.GetInputHandler().HandleInput(player->GetState(), app.GetSpeed());

    // -- Apply gravity + position --
    auto& ps = player->GetState();
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

        struct Piece {
            const char* name;
            float dx, dy;
        };
        static constexpr Piece kPieces[4] = {
            {"BrickBlockBreak_tl", -0.25f, -0.25f},
            {"BrickBlockBreak_tr", 0.25f, -0.25f},
            {"BrickBlockBreak_bl", -0.25f, 0.25f},
            {"BrickBlockBreak_br", 0.25f, 0.25f},
        };

        for (auto& p : kPieces) {
            auto debris = Mario::EntityFactory::SpawnEntity(
                level->GetEntityDefByName(p.name), bx + p.dx * bs,
                by + p.dy * bs, 0, true);
            if (debris) app.GetEntities().push_back(debris);
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
}

// ============================================================================
// CheckPipeCollision — owned by PlayingSceneHandler (PLAYING state only)
// ============================================================================
void PlayingSceneHandler::CheckPipeCollision(App& app) const {
    auto& player = app.GetPlayer();
    if (!player || player->GetState().IsDead()) return;
    if (app.GetLevelCompleteCtrl().IsActive()) return;

    Mario::PlayerState& ps = player->GetState();
    Mario::AABB playerBox = ps.GetHitbox();

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

    // Down pipe: need both halves + player centered + pressing Down
    if (pipeDown1 && pipeDown2 &&
        Util::Input::IsKeyPressed(Util::Keycode::DOWN)) {
        float pipeCenter = pipeDX + Mario::GameConfig::TILE_SIZE;
        float playerCenter = ps.GetX() + ps.GetWidth() / 2.0f;
        if (std::abs(playerCenter - pipeCenter) <
            Mario::GameConfig::TILE_SIZE * 0.6f) {
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

    // Right pipe: need any pipe half + player grounded + pressing Right
    if ((pipeRight1 || pipeRight2) && ps.IsGrounded() &&
        Util::Input::IsKeyPressed(Util::Keycode::RIGHT)) {
        float playerBot = ps.GetY() - ps.GetHeight();
        if (playerBot < pipeRY + Mario::GameConfig::TILE_SIZE * 0.1f) {
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

    // Auto-trigger pipe warp for right-pipe blocks in 1-2
    if (app.GetGameState().GetLevelName() == "1-2") {
        Mario::AABB expandedBox = playerBox;
        expandedBox.left -= 2.0f;
        expandedBox.right += 2.0f;
        for (const auto& block : app.GetLevel()->GetAllBlocks()) {
            int id = block->GetBlockID();
            if (id == Mario::GameConfig::PIPE_RIGHT_TOP ||
                id == Mario::GameConfig::PIPE_RIGHT_BOT) {
                if (expandedBox.Intersects(block->GetAABB()) &&
                    ps.IsGrounded()) {
                    LOG_INFO("Auto-warp triggered by pipe contact at 1-2");
                    app.GetLevelCompleteCtrl().StartPipeWarp(
                        *player, "Right", block->GetWorldX(),
                        block->GetWorldY());
                    Mario::AudioManager::GetInstance().PlaySFX(
                        Mario::SFXName::Warp);
                    int time = app.GetGameState().GetTimeRemaining();
                    Mario::AudioManager::GetInstance().PlayBGM(
                        (time <= 100 && time > 0)
                            ? Mario::BGMName::IntoThePipeHurryUp
                            : Mario::BGMName::IntoThePipeTheme);
                    app.GetGameState().StopTime();
                    app.GetGameState().SetNextLevelOverride("8-4");
                    app.TransitionTo(App::State::PIPE_WARP);
                    return;
                }
            }
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
            LOG_INFO("Axe touched! Starting 8-4 ending sequence.");
            entity->GetState().SetActive(false);
            app.GetGameState().StopTime();
            Mario::AudioManager::GetInstance().StopBGM();
            Mario::AudioManager::GetInstance().PlaySFX(Mario::SFXName::Break);
            app.TransitionTo(App::State::AXE_SEQUENCE);
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
