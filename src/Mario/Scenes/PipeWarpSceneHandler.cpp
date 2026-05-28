/**
 * @file PipeWarpSceneHandler.cpp
 * @brief Pipe warp transition handler (App::State::PIPE_WARP).
 *        Handles player descending or walking right into a warp pipe and
 * transitioning levels.
 * @inheritance ISceneHandler <- PipeWarpSceneHandler
 */
#include "Mario/Scenes/PipeWarpSceneHandler.hpp"

#include "App.hpp"
#include "Mario/Core/GameConfig.hpp"
#include "Mario/Services/AudioManager.hpp"
#include "Util/Logger.hpp"

namespace Mario {

void PipeWarpSceneHandler::OnEnter(App& app) {
    m_WarpSFXPlayed = false;
    m_Phase = Phase::DESCEND;
    m_tick_count = 0;

    auto& gs = app.GetGameState();
    auto& player = app.GetPlayer();
    if (player) {
        SetupWarp(gs.GetWarpDirection(), gs.GetWarpX(), gs.GetWarpY(), *player);
    }
}

void PipeWarpSceneHandler::SetupWarp(const std::string& direction,
                                     float pipeWorldX, float pipeWorldY,
                                     Player& player) {
    m_pipe_direction = direction;
    m_pipe_x = pipeWorldX;
    m_pipe_y = pipeWorldY;
    m_tick_count = 0;

    // Disable control
    player.GetState().SetControllable(false);
    player.GetState().SetVelX(0.0f);
    player.GetState().SetVelY(0.0);

    // Drop ZIndex so Mario renders BEHIND the pipe tiles.
    player.SetZIndex(GameConfig::Z_BLOCK - 1.0f);

    const float TS = static_cast<float>(GameConfig::TILE_SIZE);
    if (direction == "Down") {
        m_Phase = Phase::DESCEND;
        m_pipe_target_y = pipeWorldY + TS * 2;
        LOG_INFO("Pipe warp DOWN setup at ({}, {})", pipeWorldX, pipeWorldY);
    } else {
        m_Phase = Phase::RIGHT;
        m_pipe_target_x = pipeWorldX + TS * 2;
        player.GetState().SetGrounded(true);
        LOG_INFO("Pipe warp RIGHT setup at ({}, {})", pipeWorldX, pipeWorldY);
    }
}

void PipeWarpSceneHandler::Update(App& app) {
    auto& player = app.GetPlayer();
    auto& level = app.GetLevel();
    if (!player || !level) return;

    if (!m_WarpSFXPlayed) {
        m_WarpSFXPlayed = true;
        LOG_DEBUG("Playing Warp SFX: Resources/Audio/SFX/20. Warp.mp3");
    }

    m_tick_count++;

    switch (m_Phase) {
        case Phase::DESCEND:
            UpdatePipeDescend(*player);
            break;
        case Phase::RIGHT:
            UpdatePipeRight(*player);
            break;
        default:
            break;
    }

    player->UpdateView(app.GetCamera().GetOffset());

    if (m_Phase == Phase::COMPLETED) {
        app.GetGameState().SavePowerState(player->GetState().GetState());

        if (app.GetGameState().HasNextLevelOverride()) {
            // CheckPipeCollision set an explicit target (e.g. 1-1 bonus
            // underground entry). AdvanceToNextLevel() consumes the override.
            app.AdvanceToNextLevel();
        } else if (!app.GetGameState().IsUnderground() && level &&
                   level->HasSubLevel()) {
            // Enter an underground sub-level from the overworld (e.g. 1-1
            // overworld pipe → 1-1u bonus area).
            app.GetGameState().SetUnderground(true);
            std::string subLevel = level->GetSubLevelName();
            LOG_INFO("Loading sub-level: {}", subLevel);
            app.LoadLevel(subLevel);
            app.StartLevel();
            app.TransitionTo(App::State::PLAYING);
        } else if (app.GetGameState().IsUnderground()) {
            // Exit an underground bonus area back to the overworld
            // (e.g. 1-1u → 1-1, resuming at pipe exit X).
            app.GetGameState().SetUnderground(false);
            std::string mainLevel = app.GetGameState().GetLevelName();
            LOG_INFO("Returning to main level: {}", mainLevel);
            app.LoadLevel(mainLevel);
            app.StartLevel();
            if (player) {
                player->GetState().SetX(level->GetPipeExitX());
                player->GetState().SetControllable(true);
                player->SetVisible(true);
            }
            app.TransitionTo(App::State::PLAYING);
        } else {
            // Level-exit pipe: no sub-level and not in a bonus underground
            // area → advance to the next level in sequence (e.g. 1-2 → 8-4).
            app.GetGameState().SetUnderground(false);
            LOG_INFO("Level-exit pipe: advancing to next level");
            app.AdvanceToNextLevel();
        }
    }
}

void PipeWarpSceneHandler::OnRender(App& app) {
    app.ApplyBackground();
    app.GetRenderer().Update();
    app.GetUIManager().Update(Mario::UIManager::State::PLAYING);
}

void PipeWarpSceneHandler::UpdatePipeDescend(Player& player) {
    PlayerState& ps = player.GetState();

    float newY = ps.GetY() + GameConfig::PIPE_ANIM_SPEED;
    ps.SetY(newY);

    if (ps.GetY() > m_pipe_y + GameConfig::TILE_SIZE) {
        player.SetVisible(false);
    }

    if (ps.GetY() > m_pipe_target_y) {
        m_Phase = Phase::COMPLETED;
        LOG_DEBUG("Pipe descend complete");
    }
}

void PipeWarpSceneHandler::UpdatePipeRight(Player& player) {
    PlayerState& ps = player.GetState();

    float newX = ps.GetX() + GameConfig::PIPE_ANIM_SPEED;
    ps.SetX(newX);
    ps.SetMovingRight(true);
    ps.SetFacingRight(true);

    if (ps.GetX() > m_pipe_x + GameConfig::TILE_SIZE) {
        player.SetVisible(false);
    }

    if (ps.GetX() > m_pipe_target_x) {
        m_Phase = Phase::COMPLETED;
        LOG_DEBUG("Pipe right-walk complete");
    }
}

}  // namespace Mario
