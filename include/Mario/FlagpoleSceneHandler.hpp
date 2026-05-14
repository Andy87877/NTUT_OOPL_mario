/**
 * @file FlagpoleSceneHandler.hpp
 * @brief Flagpole ending sequence handler (App::State::FLAGPOLE).
 *        Delegates to LevelCompleteController, then advances the level.
 * @inheritance ISceneHandler <- FlagpoleSceneHandler
 */
#ifndef MARIO_FLAGPOLE_SCENE_HANDLER_HPP
#define MARIO_FLAGPOLE_SCENE_HANDLER_HPP

#include "Mario/ISceneHandler.hpp"

namespace Mario {

class FlagpoleSceneHandler : public ISceneHandler {
   public:
    FlagpoleSceneHandler() = default;

    void Update(App& app) override;
    void OnRender(App& app) override;
    const char* GetName() const override { return "FlagpoleScene"; }
};

}  // namespace Mario

#endif  // MARIO_FLAGPOLE_SCENE_HANDLER_HPP
