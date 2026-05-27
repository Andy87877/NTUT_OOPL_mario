/**
 * @file FlagpoleSceneHandler.hpp
 * @brief Flagpole ending sequence handler (App::State::FLAGPOLE).
 *        Drives the flagpole sliding, walking to castle, and level transition.
 * @inheritance ISceneHandler <- FlagpoleSceneHandler
 */
#ifndef MARIO_FLAGPOLE_SCENE_HANDLER_HPP
#define MARIO_FLAGPOLE_SCENE_HANDLER_HPP

#include <memory>
#include "Mario/Scenes/ISceneHandler.hpp"
#include "Mario/Level/Entity.hpp"
#include "Mario/Level/Block.hpp"

namespace Mario {

class FlagpoleSceneHandler : public ISceneHandler {
   public:
    enum class Phase {
        POLE_SLIDE,
        POLE_WALK,
        ENTER_CASTLE,
        WAIT_TRANSITION,
        COMPLETED
    };

    FlagpoleSceneHandler() = default;

    void OnEnter(App& app) override;
    void Update(App& app) override;
    void OnRender(App& app) override;
    const char* GetName() const override { return "FlagpoleScene"; }

    Phase GetPhase() const { return m_Phase; }
    bool IsCompleted() const { return m_Phase == Phase::COMPLETED; }

   private:
    Phase m_Phase = Phase::POLE_SLIDE;
    std::shared_ptr<Entity> m_flag_entity = nullptr;
    float m_goal_block_x = 0.0f;
    float m_goal_block_y = 0.0f;
    int m_tick_count = 0;
    int m_level_timer = -1;

    void UpdatePoleSlide(Player& player);
    void UpdatePoleWalk(Player& player, Level& level);
    void UpdateEnterCastle(Player& player);
};

}  // namespace Mario

#endif  // MARIO_FLAGPOLE_SCENE_HANDLER_HPP
