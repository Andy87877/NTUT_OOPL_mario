/**
 * @file PipeWarpSceneHandler.hpp
 * @brief Pipe warp transition handler (App::State::PIPE_WARP).
 *        Plays warp SFX once, handles player descending or walking right into pipe,
 *        then loads the destination level (sub-level, main level, or override).
 * @inheritance ISceneHandler <- PipeWarpSceneHandler
 */
#ifndef MARIO_PIPE_WARP_SCENE_HANDLER_HPP
#define MARIO_PIPE_WARP_SCENE_HANDLER_HPP

#include <string>
#include "Mario/Scenes/ISceneHandler.hpp"
#include "Mario/Player/Player.hpp"

namespace Mario {

class PipeWarpSceneHandler : public ISceneHandler {
   public:
    enum class Phase {
        DESCEND,
        RIGHT,
        COMPLETED
    };

    PipeWarpSceneHandler() = default;

    void OnEnter(App& app) override;
    void Update(App& app) override;
    void OnRender(App& app) override;
    const char* GetName() const override { return "PipeWarpScene"; }

    Phase GetPhase() const { return m_Phase; }
    bool IsCompleted() const { return m_Phase == Phase::COMPLETED; }

   private:
    /** Setup coordinates and direction for the warp sequence. */
    void SetupWarp(const std::string& direction, float pipeWorldX, float pipeWorldY, Player& player);

    Phase m_Phase = Phase::DESCEND;
    bool m_WarpSFXPlayed = false;
    std::string m_pipe_direction;
    float m_pipe_x = 0.0f;
    float m_pipe_y = 0.0f;
    float m_pipe_target_x = 0.0f;
    float m_pipe_target_y = 0.0f;
    int m_tick_count = 0;

    void UpdatePipeDescend(Player& player);
    void UpdatePipeRight(Player& player);
};

}  // namespace Mario

#endif  // MARIO_PIPE_WARP_SCENE_HANDLER_HPP
