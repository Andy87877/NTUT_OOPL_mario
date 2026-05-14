/**
 * @file PipeWarpSceneHandler.hpp
 * @brief Pipe warp transition handler (App::State::PIPE_WARP).
 *        Plays warp SFX once, drives LevelCompleteController, then loads
 *        the destination level (sub-level, main level, or override).
 * @inheritance ISceneHandler <- PipeWarpSceneHandler
 */
#ifndef MARIO_PIPE_WARP_SCENE_HANDLER_HPP
#define MARIO_PIPE_WARP_SCENE_HANDLER_HPP

#include "Mario/ISceneHandler.hpp"

namespace Mario {

class PipeWarpSceneHandler : public ISceneHandler {
   public:
    PipeWarpSceneHandler() = default;

    void OnEnter(App& app) override;
    void Update(App& app) override;
    void OnRender(App& app) override;
    const char* GetName() const override { return "PipeWarpScene"; }

   private:
    bool m_WarpSFXPlayed = false;
};

}  // namespace Mario

#endif  // MARIO_PIPE_WARP_SCENE_HANDLER_HPP
