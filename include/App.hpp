/**
 * @file App.hpp
 * @brief Main application controller for Super Mario Bros.
 *        Manages the game state machine (Title, Loading, Playing, Death,
 * GameOver) and coordinates all subsystems (Level, Player, Input, Collision,
 * Camera).
 * @inheritance None (top-level controller)
 */
#ifndef APP_HPP
#define APP_HPP

#include <memory>
#include <string>
#include <vector>

#include "Mario/Camera.hpp"
#include "Mario/CollisionManager.hpp"
#include "Mario/Entity.hpp"
#include "Mario/EntityFactory.hpp"
#include "Mario/GameConfig.hpp"
#include "Mario/GameStateManager.hpp"
#include "Mario/InputHandler.hpp"
#include "Mario/Level.hpp"
#include "Mario/LevelCompleteController.hpp"
#include "Mario/Player.hpp"
#include "Mario/UIManager.hpp"
#include "Util/Renderer.hpp"
#include "pch.hpp"  // IWYU pragma: export

class App {
   public:
    /**
     * Game state machine states.
     */
    enum class State {
        START,      // Initial boot
        TITLE,      // Title screen
        LOADING,    // Loading screen (shows world/lives)
        PLAYING,    // Active gameplay
        FLAGPOLE,   // Flagpole ending sequence
        PIPE_WARP,  // Pipe warp transition
        DEATH,      // Mario death animation
        GAME_OVER,  // Game over screen
        ESC_MENU,   // Pause menu with level skip
        END,        // Exiting application
    };

    State GetCurrentState() const { return m_CurrentState; }

    void Start();
    void Update();
    void End();  // NOLINT(readability-convert-member-functions-to-static)

   private:
    // -- State Machine --
    void UpdateTitle();
    void UpdateLoading();
    void UpdatePlaying();
    void UpdateFlagpole();
    void UpdatePipeWarp();
    void UpdateDeath();
    void UpdateGameOver();
    void UpdateESCMenu();

    // -- Level Management --
    void LoadLevel(const std::string& levelName);
    void StartLevel();
    void RenderAll();

    // -- Helpers --
    void AdvanceToNextLevel();
    void CheckEntityBlockCollision(Mario::Entity& entity);
    void CheckPlayerEntityCollision();
    void CheckFlagpoleCollision();
    void CheckPipeCollision();
    void CleanupDeadEntities();

   private:
    State m_CurrentState = State::START;

    // -- Camera --
    Mario::Camera m_Camera;

    // -- Renderer --
    Util::Renderer m_Renderer;

    // -- Level Data --
    std::shared_ptr<Mario::Level> m_Level;

    // -- Player (View, inherits Util::GameObject) --
    std::shared_ptr<Mario::Player> m_Player;

    // -- Input (Controller in MVC) --
    Mario::InputHandler m_InputHandler;

    // -- Collision Manager --
    Mario::CollisionManager m_CollisionManager;

    // -- Entities (Goomba, KoopaTroopa, Mushroom, etc.) --
    std::vector<std::shared_ptr<Mario::Entity>> m_Entities;

    // -- Phase 5: Level Completion --
    Mario::LevelCompleteController m_LevelCompleteCtrl;
    Mario::GameStateManager m_GameState;
    std::shared_ptr<Mario::Entity> m_FlagEntity;  // The flag that slides down

    // -- Phase 5: UI Manager --
    std::unique_ptr<Mario::UIManager> m_UIManager;

    // -- Movement Speed (matching C# speed calculation) --
    float m_Speed = Mario::GameConfig::SCALED_SPEED;

    // -- Loading State --
    bool m_Loading = false;
    int m_LoadTimer = -1;

    // -- Death State --
    int m_DeathTimer = -1;
    int m_Timer = 0;

    // -- ESC Menu State --
    int m_ESCMenuSelection = 0;  // 0=Resume, 1=1-1, 2=1-2, 3=8-4
};

#endif  // APP_HPP
