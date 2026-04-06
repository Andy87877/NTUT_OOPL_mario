/**
 * @file App.cpp
 * @brief Implementation of the main application controller.
 *        Handles game state machine, integrates Level loading, Player MVC,
 *        collision detection, and camera following.
 *        Game loop logic ported from C# Form1.cs onTick().
 * @inheritance None (top-level controller)
 */
#include "App.hpp"

#include "Util/Image.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include "Util/Logger.hpp"

// ============================================================================
// Start: Initialize game, transition to TITLE
// ============================================================================
void App::Start() {
    LOG_TRACE("Start");
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

    // Render the scene
    RenderAll();

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
        m_TimeSubCounter = 0;
        m_CurrentState = State::LOADING;
        m_Loading = false;
        LOG_INFO("Starting game - entering LOADING state");
    }

    // ESC to quit from title
    if (Util::Input::IsKeyDown(Util::Keycode::ESCAPE)) {
        m_CurrentState = State::END;
    }
}

void App::UpdateLoading() {
    if (!m_Loading) {
        m_Loading = true;
        m_LoadTimer = m_Timer + 50;

        std::string levelName = std::to_string(m_WorldNum) + "-" +
                                std::to_string(m_LevelNum);
        LOG_INFO("Loading World {}", levelName);
        LoadLevel(levelName);
    }

    if (m_Loading && m_LoadTimer < m_Timer) {
        m_Loading = false;
        StartLevel();
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

    if (!m_Player || !m_Level) return;

    // -- Controller: Handle input (MVC Controller layer) --
    m_InputHandler.HandleInput(m_Player->GetState(), m_Speed);

    // -- Physics & Collision --
    m_CollisionManager.CheckPlayerBlockCollision(*m_Player, *m_Level,
                                                  m_Camera.GetOffset());

    // -- Update player state (Model tick) --
    m_Player->GetState().Tick();

    // -- Camera follows player --
    m_Camera.Update(m_Player->GetWorldX(), m_Level->GetWidthPixels());

    // -- Update level blocks (animations, camera-based positioning) --
    m_Level->UpdateBlocks(m_Camera.GetOffset());

    // -- Update player view (sprite selection, screen position) --
    m_Player->UpdateView(m_Camera.GetOffset());

    // -- Timer (matching C# reference: 40 ticks = 1 game-second) --
    if (m_Player->GetState().IsControllable()) {
        m_TimeSubCounter++;
        if (m_TimeSubCounter >= Mario::GameConfig::TIME_SUB_LIMIT) {
            m_TimeCounter--;
            m_TimeSubCounter = 0;
        }
        if (m_TimeCounter <= 0) {
            m_Lives--;
            m_Player->GetState().SetDead(true);
        }
    }

    // -- Check pit fall --
    if (m_CollisionManager.CheckPitFall(*m_Player)) {
        m_Lives--;
        m_Player->GetState().SetDead(true);
        m_Player->GetState().SetControllable(false);
    }

    // -- Check death state transition --
    if (m_Player->GetState().IsDead()) {
        m_DeathTimer = m_Timer + 80; // ~1.6s death animation
        m_CurrentState = State::DEATH;
        LOG_INFO("Player died - entering DEATH state (Lives: {})", m_Lives);
    }
}

void App::UpdateDeath() {
    // Wait for death animation timer
    if (m_Timer > m_DeathTimer) {
        if (m_Lives > 0) {
            m_CurrentState = State::LOADING;
            m_Loading = false;
        } else {
            m_CurrentState = State::GAME_OVER;
            LOG_INFO("No lives remaining - GAME_OVER");
        }
    }
}

void App::UpdateGameOver() {
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
            case 0:
                m_CurrentState = State::PLAYING;
                LOG_INFO("Resuming game");
                break;
            case 1:
                m_WorldNum = 1; m_LevelNum = 1;
                m_CurrentState = State::LOADING;
                m_Loading = false;
                LOG_INFO("Jumping to World 1-1");
                break;
            case 2:
                m_WorldNum = 1; m_LevelNum = 2;
                m_CurrentState = State::LOADING;
                m_Loading = false;
                LOG_INFO("Jumping to World 1-2");
                break;
            case 3:
                m_WorldNum = 8; m_LevelNum = 4;
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

// ============================================================================
// Level Management
// ============================================================================

void App::LoadLevel(const std::string& levelName) {
    m_Camera.Reset();

    // Create and load the level
    m_Level = std::make_shared<Mario::Level>();
    if (!m_Level->Load(levelName)) {
        LOG_ERROR("Failed to load level: {}", levelName);
        m_CurrentState = State::TITLE;
        return;
    }

    // Create player at spawn position from level CSV
    float spawnX = m_Level->GetPlayerSpawnX();
    float spawnY = m_Level->GetPlayerSpawnY();
    m_Player = std::make_shared<Mario::Player>(spawnX, spawnY, 0);

    // Build renderer: clear and add all blocks + player
    m_Renderer = Util::Renderer();
    for (const auto& block : m_Level->GetAllBlocks()) {
        m_Renderer.AddChild(block);
    }
    m_Renderer.AddChild(m_Player);

    LOG_INFO("Level {} loaded with {} blocks, player at ({}, {})",
             levelName, m_Level->GetAllBlocks().size(), spawnX, spawnY);
}

void App::StartLevel() {
    m_TimeCounter = Mario::GameConfig::INITIAL_TIME;
    m_TimeSubCounter = 0;

    if (m_Player) {
        m_Player->GetState().SetControllable(true);
    }
}

void App::RenderAll() {
    m_Renderer.Update();
}

void App::AdvanceToNextLevel() {
    if (m_WorldNum == 1 && m_LevelNum == 1) {
        m_LevelNum = 2;
    } else if (m_WorldNum == 1 && m_LevelNum == 2) {
        m_WorldNum = 8;
        m_LevelNum = 4;
    } else {
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
void App::End() {
    LOG_TRACE("End");
}
