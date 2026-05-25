/**
 * @file PipeWarpSceneHandler.cpp
 * @brief Pipe warp transition handler implementation.
 *        Logic moved from App::UpdatePipeWarp().
 * @inheritance ISceneHandler <- PipeWarpSceneHandler
 */
#include "Mario/Scenes/PipeWarpSceneHandler.hpp"

#include "App.hpp"
#include "Util/Logger.hpp"

namespace Mario {

void PipeWarpSceneHandler::OnEnter(App& /*app*/) { m_WarpSFXPlayed = false; }

void PipeWarpSceneHandler::Update(App& app) {
    auto& player = app.GetPlayer();
    auto& level = app.GetLevel();
    if (!player || !level) return;

    if (!m_WarpSFXPlayed) {
        m_WarpSFXPlayed = true;
        LOG_DEBUG("Playing Warp SFX: Resources/Audio/SFX/20. Warp.mp3");
    }

    bool stillRunning = app.GetLevelCompleteCtrl().Update(
        *player, *level, app.GetCamera().GetOffset());

    if (!stillRunning && app.GetLevelCompleteCtrl().IsCompleted()) {
        app.GetGameState().SavePowerState(player->GetState().GetState());

        if (app.GetGameState().HasNextLevelOverride()) {
            // Warp to a completely different level (e.g. 1-2 -> 8-4)
            app.AdvanceToNextLevel();
        } else if (!app.GetGameState().IsUnderground()) {
            // Enter sub-level (underground)
            app.GetGameState().SetUnderground(true);
            std::string subLevel = level->GetSubLevelName();
            LOG_INFO("Loading sub-level: {}", subLevel);
            app.LoadLevel(subLevel);
            app.StartLevel();
            app.TransitionTo(App::State::PLAYING);
        } else {
            // Return from underground to main level
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
        }
    }
}

void PipeWarpSceneHandler::OnRender(App& app) {
    app.ApplyBackground();
    app.GetRenderer().Update();
    app.GetUIManager().Update(Mario::UIManager::State::PLAYING);
}

}  // namespace Mario
