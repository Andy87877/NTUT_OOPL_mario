/**
 * @file App.hpp
 * @brief Main application controller for Super Mario Bros.
 *        Owns all game subsystems and coordinates them through the State
 *        Pattern: each App::State has a dedicated ISceneHandler that drives
 *        both update and render logic for that state.
 *
 *        App::Update() is a two-liner:
 *          m_CurrentHandler->Update(*this);   // game logic
 *          m_CurrentHandler->OnRender(*this); // drawing
 *
 *        Adding a new game state requires ONLY:
 *          1. A new ISceneHandler subclass (.hpp + .cpp)
 *          2. One case in CreateSceneHandler()
 *          3. One entry in the State enum
 *        Zero changes to this file.
 *
 * @inheritance None (top-level controller)
 */
#ifndef APP_HPP
#define APP_HPP

#include <memory>
#include <string>
#include <vector>

#include "Mario/AudioManager.hpp"
#include "Mario/Camera.hpp"
#include "Mario/CollisionManager.hpp"
#include "Mario/Entity.hpp"
#include "Mario/EntityFactory.hpp"
#include "Mario/GameConfig.hpp"
#include "Mario/GameStateManager.hpp"
#include "Mario/ISceneHandler.hpp"
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
     * Each state maps to an ISceneHandler subclass created by
     * CreateSceneHandler().
     */
    enum class State {
        START,         // Initial boot
        TITLE,         // Title screen
        LOADING,       // Loading screen (shows world/lives)
        PLAYING,       // Active gameplay
        FLAGPOLE,      // Flagpole ending sequence
        PIPE_WARP,     // Pipe warp transition
        AXE_SEQUENCE,  // 8-4 boss-defeat cutscene
        DEATH,         // Mario death animation
        GAME_OVER,     // Game over screen (lives = 0)
        GAME_WON,      // All levels cleared
        ESC_MENU,      // Pause menu
        END,           // Application exit
    };

    State GetCurrentState() const { return m_CurrentState; }

    void Start();
    void Update();
    void End();  // NOLINT(readability-convert-member-functions-to-static)

    // =========================================================================
    // Public API — used by ISceneHandler subclasses
    // =========================================================================

    /** Swap the active scene handler (calls OnExit/OnEnter). */
    void TransitionTo(State next);

    // -- Level management --
    void LoadLevel(const std::string& levelName);
    void StartLevel();
    void PlayCurrentBGM();
    void AdvanceToNextLevel();

    // -- Collision helpers (called ONLY by PlayingSceneHandler — see there) --
    // NOTE: CheckFlagpoleCollision, CheckPipeCollision, CheckAxeCollision,
    //       CheckPlayerEntityCollision, CheckEntityEntityCollision, and
    //       CleanupDeadEntities are now private methods of PlayingSceneHandler.
    //       App no longer owns game-logic decisions.

    /**
     * Returns true when the current level uses a dark (underground/castle)
     * background.  Combines the runtime pipe-warp underground flag from
     * GameStateManager with Level::IsUnderground() (name-based detection).
     * Scene handlers should call this rather than duplicating the check.
     */
    bool IsUnderground() const;

    /**
     * Set OpenGL background clear color from the current level context.
     * Convenience overload — calls ApplyBackground(IsUnderground()).
     * Scene handlers should prefer this form.
     */
    void ApplyBackground();

    /**
     * Set OpenGL background clear color for the given context.
     * @param isUnderground  true = black (castle/underground), false = sky blue
     */
    void ApplyBackground(bool isUnderground);

    // -- Data accessors (return by reference for handler read/write) --
    std::shared_ptr<Mario::Player>& GetPlayer() { return m_Player; }
    std::shared_ptr<Mario::Level>& GetLevel() { return m_Level; }
    std::vector<std::shared_ptr<Mario::Entity>>& GetEntities() {
        return m_Entities;
    }

    /**
     * Add a dynamically-spawned entity to both the entity list and the
     * renderer.  Use this instead of GetEntities().push_back() for any entity
     * created after LoadLevel() (e.g. brick debris, fireballs).
     */
    void AddEntityToGame(std::shared_ptr<Mario::Entity> entity);

    Mario::Camera& GetCamera() { return m_Camera; }
    Util::Renderer& GetRenderer() { return m_Renderer; }
    Mario::GameStateManager& GetGameState() { return m_GameState; }
    Mario::UIManager& GetUIManager() { return *m_UIManager; }
    Mario::CollisionManager& GetCollisionManager() {
        return m_CollisionManager;
    }
    Mario::InputHandler& GetInputHandler() { return m_InputHandler; }
    Mario::LevelCompleteController& GetLevelCompleteCtrl() {
        return m_LevelCompleteCtrl;
    }
    std::shared_ptr<Mario::Entity>& GetFlagEntity() { return m_FlagEntity; }

    int GetTimer() const { return m_Timer; }
    float GetSpeed() const { return m_Speed; }
    const std::string& GetCurrentLevelName() const {
        return m_CurrentLevelName;
    }

    // Mutable primitive accessors (handlers write these directly)
    bool& GetLoading() { return m_Loading; }
    int& GetLoadTimer() { return m_LoadTimer; }
    int& GetDeathTimer() { return m_DeathTimer; }
    int& GetESCMenuSelection() { return m_ESCMenuSelection; }

   private:
    /** Factory: construct the ISceneHandler for a given state. */
    std::unique_ptr<Mario::ISceneHandler> CreateSceneHandler(State s);

    // -- State machine --
    State m_CurrentState = State::START;
    std::unique_ptr<Mario::ISceneHandler> m_CurrentHandler;

    // -- Camera --
    Mario::Camera m_Camera;

    // -- Renderer --
    Util::Renderer m_Renderer;

    // -- Level data --
    std::shared_ptr<Mario::Level> m_Level;

    // -- Player --
    std::shared_ptr<Mario::Player> m_Player;

    // -- Input (MVC Controller) --
    Mario::InputHandler m_InputHandler;

    // -- Collision --
    Mario::CollisionManager m_CollisionManager;

    // -- Entities --
    std::vector<std::shared_ptr<Mario::Entity>> m_Entities;

    // -- Level-completion sequences --
    Mario::LevelCompleteController m_LevelCompleteCtrl;
    Mario::GameStateManager m_GameState;
    std::shared_ptr<Mario::Entity> m_FlagEntity;

    // -- UI --
    std::unique_ptr<Mario::UIManager> m_UIManager;

    // -- Movement speed --
    float m_Speed = Mario::GameConfig::SCALED_SPEED;

    // -- Loading state --
    bool m_Loading = false;
    int m_LoadTimer = -1;

    // -- Death state --
    int m_DeathTimer = -1;

    // -- Level info --
    std::string m_CurrentLevelName;
    int m_Timer = 0;

    // -- ESC menu --
    int m_ESCMenuSelection = 0;
};

#endif  // APP_HPP
