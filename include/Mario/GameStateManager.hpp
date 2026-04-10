/**
 * @file GameStateManager.hpp
 * @brief Manages global game state: score, lives, coins, time, and level
 * progression. Ported from C# Form1.cs UI variables (lines 88-100, 124-131).
 * @inheritance None (Service class)
 */
#ifndef MARIO_GAME_STATE_MANAGER_HPP
#define MARIO_GAME_STATE_MANAGER_HPP

#include <string>
#include <vector>

#include "Mario/GameConfig.hpp"

namespace Mario {

/**
 * Manages the global game state that persists across level loads.
 * Implements the score/lives/coin/time system from C# Form1.cs.
 */
class GameStateManager {
   public:
    GameStateManager();

    /**
     * Reset all state for a new game.
     */
    void NewGame();

    /**
     * Called once per game tick to update the timer.
     */
    void Tick();

    // -- Score --
    void AddScore(int points) { m_Score += points; }
    int GetScore() const { return m_Score; }

    // -- Coins --
    void AddCoin() {
        m_Coins++;
        if (m_Coins >= 100) {
            m_Coins -= 100;
            AddLife();
        }
    }
    int GetCoins() const { return m_Coins; }

    // -- Lives --
    void AddLife() { m_Lives++; }
    void LoseLife() { m_Lives--; }
    int GetLives() const { return m_Lives; }
    bool IsGameOver() const { return m_Lives <= 0; }

    // -- Time --
    int GetTimeRemaining() const { return m_TimeCounter; }
    bool IsTimeUp() const { return m_TimeCounter <= 0; }
    void ResetTime() {
        m_TimeCounter = GameConfig::INITIAL_TIME;
        m_TimeSubCounter = 0;
    }
    void StopTime() { m_TimerRunning = false; }
    void StartTime() { m_TimerRunning = true; }

    // -- Level Progression --
    // Level order: 1-1 -> 1-2 -> 8-4
    int GetWorldNum() const { return m_WorldNum; }
    int GetLevelNum() const { return m_LevelNum; }
    std::string GetLevelName() const;

    bool IsUnderground() const { return m_IsUnderground; }
    void SetUnderground(bool v) { m_IsUnderground = v; }

    /**
     * Advance to the next level in the sequence.
     * @return The new level name (e.g. "1-2", "8-4")
     */
    std::string AdvanceLevel();

    /**
     * Jump to a specific level (for ESC menu).
     */
    void SetLevel(int world, int level);

    /**
     * Override the next level to jump to (used by warp pipes).
     * @param levelName The level name (e.g., "8-4")
     */
    void SetNextLevelOverride(const std::string& levelName) {
        m_NextLevelOverride = levelName;
    }
    bool HasNextLevelOverride() const { return !m_NextLevelOverride.empty(); }

    /**
     * Get the current player state to preserve across levels.
     */
    int GetSavedPowerState() const { return m_SavedPowerState; }
    void SavePowerState(int state) { m_SavedPowerState = state; }

    /**
     * Whether the game has been won (ended 8-4).
     */
    bool IsGameWon() const { return m_GameWon; }
    void SetGameWon(bool v) { m_GameWon = v; }

   private:
    int m_Score = 0;
    int m_Coins = 0;
    int m_Lives = GameConfig::INITIAL_LIVES;

    int m_TimeCounter = GameConfig::INITIAL_TIME;
    int m_TimeSubCounter = 0;
    bool m_TimerRunning = false;

    int m_WorldNum = 1;
    int m_LevelNum = 1;
    bool m_IsUnderground = false;
    int m_SavedPowerState = 0;
    bool m_GameWon = false;
    std::string m_NextLevelOverride = "";

    // Level sequence
    struct LevelEntry {
        int world;
        int level;
    };
    static const std::vector<LevelEntry> LEVEL_SEQUENCE;
    int m_LevelIndex = 0;
};

}  // namespace Mario

#endif  // MARIO_GAME_STATE_MANAGER_HPP
