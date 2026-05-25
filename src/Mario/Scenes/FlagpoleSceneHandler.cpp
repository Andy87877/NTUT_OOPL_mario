/**
 * @file FlagpoleSceneHandler.cpp
 * @brief Flagpole ending sequence handler implementation.
 *        Logic moved from App::UpdateFlagpole().
 * @inheritance ISceneHandler <- FlagpoleSceneHandler
 */
#include "Mario/Scenes/FlagpoleSceneHandler.hpp"

#include "App.hpp"
#include "Util/Logger.hpp"

namespace Mario {

void FlagpoleSceneHandler::Update(App& app) {
    auto& player = app.GetPlayer();
    auto& level = app.GetLevel();
    if (!player || !level) return;

    bool stillRunning = app.GetLevelCompleteCtrl().Update(
        *player, *level, app.GetCamera().GetOffset());

    app.GetCamera().Update(player->GetWorldX(), level->GetWidthPixels(),
                           app.GetCurrentLevelName(), true);
    level->UpdateBlocks(app.GetCamera().GetOffset());

    auto& flag = app.GetFlagEntity();
    if (flag && flag->GetState().IsActive()) {
        flag->UpdateView(app.GetCamera().GetOffset());
    }

    if (!stillRunning && app.GetLevelCompleteCtrl().IsCompleted()) {
        app.GetGameState().SavePowerState(player->GetState().GetState());
        app.AdvanceToNextLevel();
    }
}

void FlagpoleSceneHandler::OnRender(App& app) {
    app.ApplyBackground();
    app.GetRenderer().Update();
    app.GetUIManager().Update(Mario::UIManager::State::PLAYING);
}

}  // namespace Mario
