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

#include "Mario/Core/GameConfig.hpp"

namespace Mario {

/**
 * Manages the global game state that persists across level loads.
 * Implements the score/lives/coin/time system from C# Form1.cs.
 */
class GameStateManager {
   public:
    // Level sequence entry (world, level number)
    struct LevelEntry {
        int world;
        int level;
    };

    GameStateManager();

    // Reset all state for a new game.
    void NewGame();

    // Called once per game tick to update the timer.
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

    // -- Level Progression (order: 1-1 -> 1-2 -> 8-4) --
    int GetWorldNum() const { return m_WorldNum; }
    int GetLevelNum() const { return m_LevelNum; }
    std::string GetLevelName() const;

    bool IsUnderground() const { return m_IsUnderground; }
    void SetUnderground(bool v) { m_IsUnderground = v; }

    // Advance to next level in the sequence. Returns new level name.
    std::string AdvanceLevel();

    // Jump to a specific level (used by ESC menu).
    void SetLevel(int world, int level);

    // Override the next level (used by warp pipes).
    void SetNextLevelOverride(const std::string& levelName) {
        m_NextLevelOverride = levelName;
    }
    bool HasNextLevelOverride() const { return !m_NextLevelOverride.empty(); }

    // Warp pipe details (used to pass transition parameters without down-casting)
    void SetWarpInfo(const std::string& direction, float x, float y) {
        m_WarpDirection = direction;
        m_WarpX = x;
        m_WarpY = y;
    }
    const std::string& GetWarpDirection() const { return m_WarpDirection; }
    float GetWarpX() const { return m_WarpX; }
    float GetWarpY() const { return m_WarpY; }

    // Preserve player power state across level loads.
    int GetSavedPowerState() const { return m_SavedPowerState; }
    void SavePowerState(int state) { m_SavedPowerState = state; }

    // Whether the game has been won (8-4 completed).
    bool IsGameWon() const { return m_GameWon; }
    void SetGameWon(bool v = true) { m_GameWon = v; }

    // Cheat Mode (外掛模式)
    bool IsCheatModeActive() const { return m_CheatModeActive; }
    void SetCheatModeActive(bool active) { m_CheatModeActive = active; }

   private:
    // Score / coins / lives
    int m_Score = 0;
    int m_Coins = 0;
    int m_Lives = GameConfig::INITIAL_LIVES;

    // Timer
    int m_TimeCounter = GameConfig::INITIAL_TIME;
    int m_TimeSubCounter = 0;
    bool m_TimerRunning = false;

    // Level tracking
    int m_WorldNum = 1;
    int m_LevelNum = 1;
    int m_LevelIndex = 0;
    bool m_IsUnderground = false;
    std::string m_NextLevelOverride;

    // Warp pipe transition details
    std::string m_WarpDirection = "";
    float m_WarpX = 0.0f;
    float m_WarpY = 0.0f;

    // Persistence across levels
    int m_SavedPowerState = 0;  // 0=small, 1=big, 2=fire
    bool m_GameWon = false;
    bool m_CheatModeActive = false;

    static const std::vector<LevelEntry> LEVEL_SEQUENCE;
};

}  // namespace Mario

#endif  // MARIO_GAME_STATE_MANAGER_HPP
