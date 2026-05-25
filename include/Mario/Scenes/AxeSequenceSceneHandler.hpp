/**
 * @file AxeSequenceSceneHandler.hpp
 * @brief 8-4 boss-defeat ending cutscene handler (App::State::AXE_SEQUENCE).
 *        Drives LevelCompleteController through the Bowser defeat animation,
 *        then transitions to GAME_WON.
 * @inheritance ISceneHandler <- AxeSequenceSceneHandler
 */
#ifndef MARIO_AXE_SEQUENCE_SCENE_HANDLER_HPP
#define MARIO_AXE_SEQUENCE_SCENE_HANDLER_HPP

#include "Mario/Scenes/ISceneHandler.hpp"

namespace Mario {

class AxeSequenceSceneHandler : public ISceneHandler {
   public:
    AxeSequenceSceneHandler() = default;

    void Update(App& app) override;
    void OnRender(App& app) override;
    const char* GetName() const override { return "AxeSequenceScene"; }
};

}  // namespace Mario

#endif  // MARIO_AXE_SEQUENCE_SCENE_HANDLER_HPP
