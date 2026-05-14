/**
 * @file LoadingSceneHandler.cpp
 * @brief Loading screen scene handler implementation.
 * @inheritance ISceneHandler <- LoadingSceneHandler
 */
#include "Mario/LoadingSceneHandler.hpp"

#include "App.hpp"
#include "Mario/GameConfig.hpp"
#include "Util/Logger.hpp"

namespace Mario {

void LoadingSceneHandler::OnEnter(App& app) {
    // Reset loading flag so Update() triggers a fresh level load on its first
    // frame.
    app.GetLoading() = false;
}

void LoadingSceneHandler::Update(App& app) {
    bool& loading = app.GetLoading();
    if (!loading) {
        loading = true;
        app.GetLoadTimer() =
            app.GetTimer() + Mario::GameConfig::LEVEL_TRANSITION_DELAY;
        std::string levelName = app.GetGameState().GetLevelName();
        LOG_INFO("Loading World {}", levelName);
        app.LoadLevel(levelName);
    }
    if (loading && app.GetLoadTimer() < app.GetTimer()) {
        loading = false;
        app.StartLevel();
        app.TransitionTo(App::State::PLAYING);
        LOG_INFO("Level loaded - entering PLAYING state");
    }
}

void LoadingSceneHandler::OnRender(App& app) {
    const std::string& lvl = app.GetCurrentLevelName();
    bool underground = app.GetGameState().IsUnderground() ||
                       lvl.find("u") != std::string::npos || lvl == "1-2" ||
                       lvl == "8-4";
    app.ApplyBackground(underground);
    app.GetRenderer().Update();
    app.GetUIManager().Update(Mario::UIManager::State::LOADING);
}

}  // namespace Mario
