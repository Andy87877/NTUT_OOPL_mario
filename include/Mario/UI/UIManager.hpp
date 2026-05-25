/**
 * @file UIManager.hpp
 * @brief Thin dispatcher that owns one IUIPanel per game screen and a
 *        shared UI renderer.  Adding a new screen only requires a new
 *        IUIPanel subclass and one map entry — UIManager::Update() needs
 *        no modification (OCP).
 * @inheritance None (Manager class)
 */
#ifndef MARIO_UI_MANAGER_HPP
#define MARIO_UI_MANAGER_HPP

#include <chrono>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "Mario/UI/FloatingText.hpp"
#include "Mario/Level/GameStateManager.hpp"
#include "Mario/UI/UIPanel.hpp"
#include "Mario/UI/UIWidgets.hpp"
#include "Util/Renderer.hpp"
#include "config.hpp"

namespace Mario {

/**
 * Manages all UI panels, the shared UI renderer, and floating text effects.
 * Each game screen is represented by a concrete IUIPanel that owns its own
 * widgets; UIManager dispatches to the active panel via m_PanelMap.
 */
class UIManager {
   public:
    enum class State {
        TITLE,
        LOADING,
        PLAYING,
        GAME_OVER,
        GAME_WON,
        ESC_MENU,
        AXE_SEQUENCE  // 8-4 Bowser defeat ending cutscene
    };

    /**
     * Phase of the axe/ending sequence — drives what text is shown.
     * Mirrors EndingPhase in LevelCompleteController without coupling.
     */
    enum class EndingTextPhase { NONE, CREDITS };

    /** Inform UIManager which phase the ending sequence is in. */
    void SetEndingPhase(EndingTextPhase phase) { m_EndingTextPhase = phase; }

    explicit UIManager(GameStateManager* gameState);
    virtual ~UIManager() = default;

    /**
     * Dispatch to the active screen panel and flush the UI renderer.
     * @param currentState    Which screen is active.
     * @param escMenuSelection  Highlighted item index (ESC_MENU only).
     * @param powerStateName  Current cheat power name (ESC_MENU only).
     */
    void Update(State currentState, int escMenuSelection = 0,
                const std::string& powerStateName = "SMALL");

    /**
     * Add a floating score/1UP text at PTSD screen coordinates.
     * @param screenX  PTSD x (after camera offset).
     * @param screenY  PTSD y (after camera offset).
     * @param text     Content (e.g. "+200", "+1UP").
     * @param frames   Lifetime in frames (default 60 ≈ 1 s at 60 FPS).
     */
    void AddFloatingText(float screenX, float screenY, const std::string& text,
                         int frames = 60);

    GameStateManager* GetGameState() const { return m_GameState; }
    Util::Renderer& GetRenderer() { return m_UIRenderer; }

   private:
    /** Hide every scene-specific panel (not the HUD). */
    void HideAllScenePanels();

    GameStateManager* m_GameState;
    Util::Renderer m_UIRenderer;

    // Shared font settings
    std::string m_FontPath = std::string(RESOURCE_DIR) + "/Font/mario.ttf";
    int m_FontSize = 16;

    // -------------------------------------------------------------------------
    // UI panels (Strategy Pattern — one panel owns the widgets for one screen)
    // -------------------------------------------------------------------------
    HUDPanel m_HUDPanel;
    TitlePanel m_TitlePanel;
    LoadingPanel m_LoadingPanel;
    SimpleTextPanel m_GameOverPanel;
    SimpleTextPanel m_GameWonPanel;
    ESCMenuPanel m_ESCMenuPanel;
    AxeEndingPanel m_AxeEndingPanel;

    /** Maps each State to the scene-specific panel that should be active. */
    std::unordered_map<State, IUIPanel*> m_PanelMap;

    // -------------------------------------------------------------------------
    // Cross-panel features
    // -------------------------------------------------------------------------
    std::vector<std::shared_ptr<FloatingText>> m_FloatingTexts;
    EndingTextPhase m_EndingTextPhase = EndingTextPhase::NONE;

    // FPS counter
    std::shared_ptr<UIText> m_FPSText;
    int m_FPS = 0;
    int m_FrameCount = 0;
    std::chrono::steady_clock::time_point m_LastFPSTime =
        std::chrono::steady_clock::now();

    // Copyright text (always visible)
    std::shared_ptr<UIText> m_CopyrightText;
};

}  // namespace Mario

#endif  // MARIO_UI_MANAGER_HPP
