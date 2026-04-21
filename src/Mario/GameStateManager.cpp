/**
 * @file GameStateManager.cpp
 * @brief Implementation of global game state management.
 *        Handles score, lives, coins, time countdown, and level progression.
 *        Ported from C# Form1.cs variables (lines 88-100, 124-131).
 * @inheritance None (Service class)
 */
#include "Mario/GameStateManager.hpp"

#include "Util/Logger.hpp"

namespace Mario {

// Level sequence: 1-1 -> 1-2 -> 8-4
const std::vector<GameStateManager::LevelEntry>
    GameStateManager::LEVEL_SEQUENCE = {
        {1, 1},  // World 1-1
        {1, 2},  // World 1-2
        {8, 4},  // World 8-4 (final)
};

GameStateManager::GameStateManager() { NewGame(); }

void GameStateManager::NewGame() {
    m_Score = 0;
    m_Coins = 0;
    m_Lives = GameConfig::INITIAL_LIVES;
    m_TimeCounter = GameConfig::INITIAL_TIME;
    m_TimeSubCounter = 0;
    m_TimerRunning = false;
    m_WorldNum = 1;
    m_LevelNum = 1;
    m_LevelIndex = 0;
    m_SavedPowerState = 0;
    m_GameWon = false;
}

void GameStateManager::Tick() {
    if (!m_TimerRunning || m_TimeCounter <= 0) return;

    // C# Form1.cs lines 673-685: 40 ticks = 1 game second
    m_TimeSubCounter++;
    if (m_TimeSubCounter >= GameConfig::TIME_SUB_LIMIT) {
        m_TimeCounter--;
        m_TimeSubCounter = 0;
    }
}

std::string GameStateManager::GetLevelName() const {
    return std::to_string(m_WorldNum) + "-" + std::to_string(m_LevelNum);
}

std::string GameStateManager::AdvanceLevel() {
    if (!m_NextLevelOverride.empty()) {
        std::string next = m_NextLevelOverride;
        m_NextLevelOverride.clear();

        // Find and sync the index for the override
        for (int i = 0; i < static_cast<int>(LEVEL_SEQUENCE.size()); i++) {
            if (std::to_string(LEVEL_SEQUENCE[i].world) + "-" +
                    std::to_string(LEVEL_SEQUENCE[i].level) ==
                next) {
                m_WorldNum = LEVEL_SEQUENCE[i].world;
                m_LevelNum = LEVEL_SEQUENCE[i].level;
                m_LevelIndex = i;
                break;
            }
        }
        return next;
    }

    m_LevelIndex++;

    if (m_LevelIndex >= static_cast<int>(LEVEL_SEQUENCE.size())) {
        // Game completed!
        m_GameWon = true;
        LOG_INFO("Game completed! All levels beaten.");
        return "";
    }

    m_WorldNum = LEVEL_SEQUENCE[m_LevelIndex].world;
    m_LevelNum = LEVEL_SEQUENCE[m_LevelIndex].level;

    LOG_INFO("Advanced to level {}-{}", m_WorldNum, m_LevelNum);
    return GetLevelName();
}

void GameStateManager::SetLevel(int world, int level) {
    m_WorldNum = world;
    m_LevelNum = level;

    // Find the index in the sequence
    for (int i = 0; i < static_cast<int>(LEVEL_SEQUENCE.size()); i++) {
        if (LEVEL_SEQUENCE[i].world == world &&
            LEVEL_SEQUENCE[i].level == level) {
            m_LevelIndex = i;
            break;
        }
    }

    LOG_INFO("Level set to {}-{}", world, level);
}

}  // namespace Mario
