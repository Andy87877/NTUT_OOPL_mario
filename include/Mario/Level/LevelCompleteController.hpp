/**
 * @file LevelCompleteController.hpp
 * @brief Controller for level completion sequences:
 *        flagpole slide (1-1), pipe warp (1-2), and Bowser defeat (8-4).
 *        Ported from C# Form1.cs ending/pipe logic (lines 941-1265).
 * @inheritance None (Controller in MVC)
 */
#ifndef MARIO_LEVEL_COMPLETE_CONTROLLER_HPP
#define MARIO_LEVEL_COMPLETE_CONTROLLER_HPP

#include <memory>

#include "Mario/Level/Entity.hpp"
#include "Mario/Core/GameConfig.hpp"
#include "Mario/Level/Level.hpp"
#include "Mario/Player/Player.hpp"

namespace Mario {

/**
 * Phases of the level completion sequence.
 */
enum class EndingPhase {
    NONE,                // Normal gameplay
    POLE_SLIDE,          // Mario sliding down the flagpole
    POLE_WALK,           // Mario walks from pole to castle
    ENTER_CASTLE,        // Mario enters castle door (invisible)
    WAIT_TRANSITION,     // Brief pause before loading next level
    PIPE_DESCEND,        // Mario descending into pipe
    PIPE_RIGHT,          // Mario walking right into pipe
    AXE_SEQUENCE_START,  // Start of the 8-4 Bowser defeat sequence
    BRIDGE_COLLAPSE,     // Bridge collapses
    BOWSER_FALL,         // Bowser falls
    WALK_TO_PRINCESS,    // Mario walks to the Princess
    PRINCESS_DIALOG,     // Princess dialog
    COMPLETED            // Ready to advance
};

/**
 * Manages the level completion cutscene (flagpole / pipe warp).
 * Called by App when player touches a goal block or enters a pipe.
 */
class LevelCompleteController {
   public:
    LevelCompleteController() = default;

    /**
     * Start the flagpole ending sequence.
     * C# reference: Form1.cs lines 1231-1265
     * @param player The player View
     * @param flagEntity The flag entity that slides down with Mario
     * @param goalBlock The goal block (flagpole) that triggered this
     */
    void StartFlagpole(Player& player, std::shared_ptr<Entity> flagEntity,
                       const Block* goalBlock);

    /**
     * Start the 8-4 Bowser defeat sequence.
     * @param player The player View
     * @param bowser The Bowser entity
     * @param princess The Princess entity
     */
    void StartAxeSequence(Player& player, std::shared_ptr<Entity> bowser,
                          std::shared_ptr<Entity> princess);

    /**
     * Start the pipe warp sequence.
     * C# reference: Form1.cs lines 941-968
     * @param player The player View
     * @param direction "Down" or "Right"
     * @param pipeWorldX World X of the pipe entrance
     * @param pipeWorldY World Y of the pipe entrance
     */
    void StartPipeWarp(Player& player, const std::string& direction,
                       float pipeWorldX, float pipeWorldY);

    /**
     * Update the sequence each frame.
     * @param player The player View
     * @param level The current level (for castle block lookup)
     * @param cameraOffset Current camera offset
     * @return true while the sequence is still running
     */
    bool Update(Player& player, Level& level, float cameraOffset);

    /**
     * Check the current phase.
     */
    EndingPhase GetPhase() const { return m_Phase; }
    bool IsActive() const {
        return m_Phase != EndingPhase::NONE &&
               m_Phase != EndingPhase::COMPLETED;
    }
    bool IsCompleted() const { return m_Phase == EndingPhase::COMPLETED; }

    /**
     * Whether this was a pipe warp (affects next level loading).
     */
    bool WasPipeWarp() const { return m_was_pipe_warp; }

    /**
     * Reset state for a new level.
     */
    void Reset();

   private:
    EndingPhase m_Phase = EndingPhase::NONE;
    bool m_was_pipe_warp = false;
    std::shared_ptr<Entity> m_flag_entity = nullptr;
    std::shared_ptr<Entity> m_bowser = nullptr;
    std::shared_ptr<Entity> m_princess = nullptr;
    int m_end_timer = -1;
    int m_level_timer = -1;
    int m_tick_count = 0;
    float m_goal_block_x = 0.0f;
    float m_goal_block_y = 0.0f;
    std::string m_pipe_direction;
    float m_pipe_x = 0.0f;
    float m_pipe_y = 0.0f;
    float m_pipe_target_x = 0.0f;
    float m_pipe_target_y = 0.0f;

    // -- Helpers --
    void UpdatePoleSlide(Player& player);
    void UpdatePoleWalk(Player& player, Level& level);
    void UpdateEnterCastle(Player& player);
    void UpdatePipeDescend(Player& player);
    void UpdatePipeRight(Player& player);
    void UpdateAxeSequence(Player& player, Level& level);
};

}  // namespace Mario

#endif  // MARIO_LEVEL_COMPLETE_CONTROLLER_HPP
