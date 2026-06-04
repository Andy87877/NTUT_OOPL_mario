/**
 * @file GameStateManager.cpp
 * @brief Implementation of global game state management.
 *        Handles score, lives, coins, time countdown, and level progression.
 *        Ported from C# Form1.cs variables (lines 88-100, 124-131).
 * @inheritance None (Service class)
 */
#include "Mario/Level/GameStateManager.hpp"

#include <fstream>
#include <sstream>
#include "Util/Logger.hpp"

namespace Mario {

GameStateManager::GameStateManager() {
    LoadLevelSequence();
    NewGame();
}

void GameStateManager::LoadLevelSequence() {
    m_LevelSequence.clear();
    std::string path = std::string(RESOURCE_DIR) + "/Levels/LevelSequence.csv";
    std::ifstream file(path);
    if (!file.is_open()) {
        LOG_ERROR("Cannot open LevelSequence.csv: {}, using default fallback", path);
        m_LevelSequence = {{1, 1}, {1, 2}, {8, 4}};
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty() || line[0] == '#') continue; // Skip comments and empty lines

        std::stringstream ss(line);
        std::string wStr, lStr;
        if (std::getline(ss, wStr, ',') && std::getline(ss, lStr, ',')) {
            try {
                int world = std::stoi(wStr);
                int level = std::stoi(lStr);
                m_LevelSequence.push_back({world, level});
            } catch (...) {
                LOG_ERROR("Invalid line in LevelSequence.csv: {}", line);
            }
        }
    }

    if (m_LevelSequence.empty()) {
        m_LevelSequence = {{1, 1}, {1, 2}, {8, 4}};
    }
    LOG_INFO("Loaded {} levels in sequence from LevelSequence.csv", m_LevelSequence.size());
}

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
    m_CheatModeActive = false;
    m_WarpDirection = "";
    m_WarpX = 0.0f;
    m_WarpY = 0.0f;
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
        for (int i = 0; i < static_cast<int>(m_LevelSequence.size()); i++) {
            if (std::to_string(m_LevelSequence[i].world) + "-" +
                    std::to_string(m_LevelSequence[i].level) ==
                next) {
                m_WorldNum = m_LevelSequence[i].world;
                m_LevelNum = m_LevelSequence[i].level;
                m_LevelIndex = i;
                break;
            }
        }
        return next;
    }

    m_LevelIndex++;

    if (m_LevelIndex >= static_cast<int>(m_LevelSequence.size())) {
        // Game completed!
        m_GameWon = true;
        LOG_INFO("Game completed! All levels beaten.");
        return "";
    }

    m_WorldNum = m_LevelSequence[m_LevelIndex].world;
    m_LevelNum = m_LevelSequence[m_LevelIndex].level;

    LOG_INFO("Advanced to level {}-{}", m_WorldNum, m_LevelNum);
    return GetLevelName();
}

void GameStateManager::SetLevel(int world, int level) {
    m_WorldNum = world;
    m_LevelNum = level;

    // Find the index in the sequence
    for (int i = 0; i < static_cast<int>(m_LevelSequence.size()); i++) {
        if (m_LevelSequence[i].world == world &&
            m_LevelSequence[i].level == level) {
            m_LevelIndex = i;
            break;
        }
    }

    LOG_INFO("Level set to {}-{}", world, level);
}

}  // namespace Mario
