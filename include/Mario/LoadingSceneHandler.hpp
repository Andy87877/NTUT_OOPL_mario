/**
 * @file LoadingSceneHandler.hpp
 * @brief Loading screen scene handler (App::State::LOADING).
 *        Loads the level data, then starts gameplay after a brief delay.
 * @inheritance ISceneHandler <- LoadingSceneHandler
 */
#ifndef MARIO_LOADING_SCENE_HANDLER_HPP
#define MARIO_LOADING_SCENE_HANDLER_HPP

#include "Mario/ISceneHandler.hpp"

namespace Mario {

class LoadingSceneHandler : public ISceneHandler {
   public:
    LoadingSceneHandler() = default;

    void OnEnter(App& app) override;
    void Update(App& app) override;
    void OnRender(App& app) override;
    const char* GetName() const override { return "LoadingScene"; }
};

}  // namespace Mario

#endif  // MARIO_LOADING_SCENE_HANDLER_HPP
