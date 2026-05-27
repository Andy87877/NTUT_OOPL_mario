/**
 * @file AxeSequenceSceneHandler.hpp
 * @brief 8-4 boss-defeat ending cutscene handler (App::State::AXE_SEQUENCE).
 *        Drives the Bowser defeat animation, bridge collapse, and walking to Princess,
 *        then transitions to GAME_WON.
 * @inheritance ISceneHandler <- AxeSequenceSceneHandler
 */
#ifndef MARIO_AXE_SEQUENCE_SCENE_HANDLER_HPP
#define MARIO_AXE_SEQUENCE_SCENE_HANDLER_HPP

#include <memory>
#include "Mario/Scenes/ISceneHandler.hpp"
#include "Mario/Level/Entity.hpp"
#include "Mario/Player/Player.hpp"

namespace Mario {

class AxeSequenceSceneHandler : public ISceneHandler {
   public:
    enum class Phase {
        START,
        BRIDGE_COLLAPSE,
        BOWSER_FALL,
        WALK_TO_PRINCESS,
        PRINCESS_DIALOG,
        COMPLETED
    };

    AxeSequenceSceneHandler() = default;

    void OnEnter(App& app) override;
    void Update(App& app) override;
    void OnRender(App& app) override;
    const char* GetName() const override { return "AxeSequenceScene"; }

    Phase GetPhase() const { return m_Phase; }
    bool IsCompleted() const { return m_Phase == Phase::COMPLETED; }

   private:
    Phase m_Phase = Phase::START;
    std::shared_ptr<Entity> m_bowser = nullptr;
    std::shared_ptr<Entity> m_princess = nullptr;
    int m_tick_count = 0;

    void UpdateAxeSequence(Player& player, Level& level);
};

}  // namespace Mario

#endif  // MARIO_AXE_SEQUENCE_SCENE_HANDLER_HPP
