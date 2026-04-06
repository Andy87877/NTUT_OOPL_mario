/**
 * @file App.cpp
 * @brief Implementation of the main application controller.
 *        Handles game state machine transitions and frame-by-frame updates.
 * @inheritance None (top-level controller)
 */
#include "App.hpp"

#include "Util/Image.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include "Util/Logger.hpp"
#include "Util/Renderer.hpp"

// ============================================================================
// Start: Initialize game, transition to TITLE
// ============================================================================
void App::Start() {
    LOG_TRACE("Start");

    // TODO Phase 2: Load title screen level CSV
    // TODO Phase 5: Initialize all managers

    m_CurrentState = State::TITLE;
    LOG_INFO("Game initialized - entering TITLE state");
}

// ============================================================================
// Update: Main loop dispatcher based on current state
// ============================================================================
void App::Update() {
    m_Timer++;

    switch (m_CurrentState) {
        case State::TITLE:
            UpdateTitle();
            break;
        case State::LOADING:
            UpdateLoading();
            break;
        case State::PLAYING:
            UpdatePlaying();
            break;
        case State::DEATH:
            UpdateDeath();
            break;
        case State::GAME_OVER:
            UpdateGameOver();
            break;
        case State::ESC_MENU:
            UpdateESCMenu();
            break;
        default:
            break;
    }

    // Global exit check
    if (Util::Input::IfExit()) {
        m_CurrentState = State::END;
    }
}

// ============================================================================
// State Handlers
// ============================================================================

void App::UpdateTitle() {
    // Press Enter to start the game
    if (Util::Input::IsKeyDown(Util::Keycode::RETURN)) {
        m_WorldNum = 1;
        m_LevelNum = 1;
        m_Lives = Mario::GameConfig::INITIAL_LIVES;
        m_Score = 0;
        m_Coins = 0;
        m_TimeCounter = Mario::GameConfig::INITIAL_TIME;
        m_CurrentState = State::LOADING;
        m_Loading = false;
        LOG_INFO("Starting game - entering LOADING state");
    }

    // ESC to quit from title
    if (Util::Input::IsKeyUp(Util::Keycode::ESCAPE)) {
        m_CurrentState = State::END;
    }
}

void App::UpdateLoading() {
    if (!m_Loading) {
        m_Loading = true;
        m_LoadTimer = m_Timer + 50; // Show loading for ~1 second
        LOG_INFO("Loading World {}-{}", m_WorldNum, m_LevelNum);
    }

    if (m_Loading && m_LoadTimer < m_Timer) {
        // TODO Phase 2: LoadLevel(levelName, 0);
        // TODO Phase 2: StartLevel();
        m_Loading = false;
        m_CurrentState = State::PLAYING;
        LOG_INFO("Level loaded - entering PLAYING state");
    }
}

void App::UpdatePlaying() {
    // ESC to open pause menu
    if (Util::Input::IsKeyDown(Util::Keycode::ESCAPE)) {
        m_ESCMenuSelection = 0;
        m_CurrentState = State::ESC_MENU;
        LOG_INFO("Game paused - entering ESC_MENU state");
        return;
    }

    // TODO Phase 3: Handle player input (move, jump, crouch, fire)
    // TODO Phase 3: Update physics
    // TODO Phase 3: Check collisions
    // TODO Phase 4: Update entities
    // TODO Phase 5: Update timer, UI, check win/death conditions
}

void App::UpdateDeath() {
    // TODO Phase 5: Play death animation, then either respawn or game over
    // Placeholder: wait a bit then go to loading or game over
    if (Util::Input::IsKeyDown(Util::Keycode::RETURN)) {
        if (m_Lives > 0) {
            m_CurrentState = State::LOADING;
            m_Loading = false;
        } else {
            m_CurrentState = State::GAME_OVER;
        }
    }
}

void App::UpdateGameOver() {
    // Press Enter to return to title
    if (Util::Input::IsKeyDown(Util::Keycode::RETURN)) {
        m_CurrentState = State::TITLE;
        LOG_INFO("Game Over - returning to TITLE");
    }
}

void App::UpdateESCMenu() {
    // Menu navigation
    if (Util::Input::IsKeyDown(Util::Keycode::UP)) {
        m_ESCMenuSelection = (m_ESCMenuSelection - 1 + 4) % 4;
    }
    if (Util::Input::IsKeyDown(Util::Keycode::DOWN)) {
        m_ESCMenuSelection = (m_ESCMenuSelection + 1) % 4;
    }

    // Select
    if (Util::Input::IsKeyDown(Util::Keycode::RETURN)) {
        switch (m_ESCMenuSelection) {
            case 0: // Resume
                m_CurrentState = State::PLAYING;
                LOG_INFO("Resuming game");
                break;
            case 1: // Jump to 1-1
                m_WorldNum = 1;
                m_LevelNum = 1;
                m_CurrentState = State::LOADING;
                m_Loading = false;
                LOG_INFO("Jumping to World 1-1");
                break;
            case 2: // Jump to 1-2
                m_WorldNum = 1;
                m_LevelNum = 2;
                m_CurrentState = State::LOADING;
                m_Loading = false;
                LOG_INFO("Jumping to World 1-2");
                break;
            case 3: // Jump to 8-4
                m_WorldNum = 8;
                m_LevelNum = 4;
                m_CurrentState = State::LOADING;
                m_Loading = false;
                LOG_INFO("Jumping to World 8-4");
                break;
        }
    }

    // ESC to resume
    if (Util::Input::IsKeyDown(Util::Keycode::ESCAPE)) {
        m_CurrentState = State::PLAYING;
        LOG_INFO("ESC pressed again - resuming game");
    }
}

void App::AdvanceToNextLevel() {
    if (m_WorldNum == 1 && m_LevelNum == 1) {
        m_LevelNum = 2; // 1-1 -> 1-2
    } else if (m_WorldNum == 1 && m_LevelNum == 2) {
        m_WorldNum = 8;
        m_LevelNum = 4; // 1-2 -> 8-4
    } else {
        // Game complete!
        m_CurrentState = State::TITLE;
        LOG_INFO("Game Complete! Returning to title.");
        return;
    }
    m_CurrentState = State::LOADING;
    m_Loading = false;
}

// ============================================================================
// End
// ============================================================================
void App::End() { // NOLINT(this method will mutate members in the future)
    LOG_TRACE("End");
}
