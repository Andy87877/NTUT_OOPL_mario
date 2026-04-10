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

#include "Mario/Entity.hpp"
#include "Mario/GameConfig.hpp"
#include "Mario/Level.hpp"
#include "Mario/Player.hpp"

namespace Mario {

/**
 * Phases of the level completion sequence.
 */
enum class EndingPhase {
    NONE,             // Normal gameplay
    POLE_SLIDE,       // Mario sliding down the flagpole
    POLE_WALK,        // Mario walks from pole to castle
    ENTER_CASTLE,     // Mario enters castle door (invisible)
    WAIT_TRANSITION,  // Brief pause before loading next level
    PIPE_DESCEND,     // Mario descending into pipe
    PIPE_RIGHT,       // Mario walking right into pipe
    COMPLETED         // Ready to advance
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
    bool WasPipeWarp() const { return m_WasPipeWarp; }

    /**
     * Reset state for a new level.
     */
    void Reset();

   private:
    EndingPhase m_Phase = EndingPhase::NONE;
    bool m_WasPipeWarp = false;
    bool m_WarpSFXPlayed = false;  // Track if warp SFX has been played

    // Flagpole state
    std::shared_ptr<Entity> m_FlagEntity;
    float m_GoalBlockX = 0.0f;
    float m_GoalBlockY = 0.0f;
    int m_EndTimer = -1;
    int m_LevelTimer = -1;
    int m_TickCount = 0;

    // Pipe state
    std::string m_PipeDirection;
    float m_PipeX = 0.0f;
    float m_PipeY = 0.0f;
    float m_PipeTargetY = 0.0f;
    float m_PipeTargetX = 0.0f;

    // -- Helpers --
    void UpdatePoleSlide(Player& player);
    void UpdatePoleWalk(Player& player, Level& level);
    void UpdateEnterCastle(Player& player);
    void UpdatePipeDescend(Player& player);
    void UpdatePipeRight(Player& player);
};

}  // namespace Mario

#endif  // MARIO_LEVEL_COMPLETE_CONTROLLER_HPP
