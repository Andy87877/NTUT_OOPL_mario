/**
 * @file UIManager.hpp
 * @brief Manager for UI state, rendering HUD and floating text effects.
 * @inheritance None (Manager class)
 */
#ifndef MARIO_UI_MANAGER_HPP
#define MARIO_UI_MANAGER_HPP

#include <memory>
#include <string>
#include <vector>

#include "Mario/CoinUI.hpp"
#include "Mario/FloatingText.hpp"
#include "Mario/GameStateManager.hpp"
#include "Mario/UIImage.hpp"
#include "Mario/UIText.hpp"
#include "Util/GameObject.hpp"
#include "Util/Image.hpp"
#include "Util/Renderer.hpp"

namespace Mario {

/**
 * Manages UI rendering and floating text effects.
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
    enum class EndingTextPhase { NONE, CREDITS };  // extend as needed

    /**
     * Inform UIManager which phase the ending sequence is in.
     * Call each frame from App::RenderAll during AXE_SEQUENCE state.
     */
    void SetEndingPhase(EndingTextPhase phase) { m_EndingTextPhase = phase; }

    UIManager(GameStateManager* gameState);
    virtual ~UIManager() = default;

    /**
     * Update floating text state and UI elements based on current game state.
     */
    void Update(State currentState, int escMenuSelection = 0);

    /**
     * Add a floating text (for points, 1UP, etc) at screen coordinates.
     * @param screenX Screen X position (PTSD coordinates, after camera offset
     * applied)
     * @param screenY Screen Y position (PTSD coordinates)
     * @param text Text to display (e.g., "+200", "+1UP")
     * @param frames Duration in frames (default 60 frames ~1 second at 60 FPS)
     */
    void AddFloatingText(float screenX, float screenY, const std::string& text,
                         int frames = 60);

    /**
     * Get game state for UI display.
     */
    GameStateManager* GetGameState() const { return m_GameState; }

    Util::Renderer& GetRenderer() { return m_UIRenderer; }

   private:
    void InitHUD();
    void InitTitleScreen();
    void InitLoadingScreen();
    void InitGameOverScreen();
    void InitESCMenu();
    void InitAxeEndingScreen();

    void UpdateHUD();
    void UpdateTitleScreen();
    void UpdateLoadingScreen();
    void UpdateGameOverScreen();
    void UpdateGameWonScreen();
    void UpdateESCMenu(int selection);
    void UpdateAxeEndingScreen();

    GameStateManager* m_GameState;
    Util::Renderer m_UIRenderer;

    // Font path and size
    std::string m_FontPath = std::string(RESOURCE_DIR) + "/Font/mario.ttf";
    int m_FontSize = 16;  // Adjusted for 1280x720 resolution

    // HUD Background (black rectangle behind HUD text)
    std::shared_ptr<Util::GameObject> m_HUDBackground;

    // HUD Text Elements
    std::shared_ptr<UIText> m_HeaderMario;
    std::shared_ptr<UIText> m_HeaderWorld;
    std::shared_ptr<UIText> m_HeaderTime;
    std::shared_ptr<UIText> m_ScoreText;
    std::shared_ptr<UIText> m_WorldText;
    std::shared_ptr<UIText> m_TimeText;

    // Coin UI (animated coin counter)
    std::shared_ptr<CoinUI> m_CoinUI;

    // Title / Loading / Game Over Text Elements
    std::shared_ptr<UIText> m_CenterLabel;
    std::shared_ptr<UIText> m_SubLabel;

    // ESC Menu Text
    std::vector<std::shared_ptr<UIText>> m_MenuTexts;

    // Floating text effects
    std::vector<std::shared_ptr<FloatingText>> m_FloatingTexts;

    // Time warning flash animation counter
    int m_FlashCounter = 0;

    // Axe ending sequence text elements
    EndingTextPhase m_EndingTextPhase = EndingTextPhase::NONE;
    std::shared_ptr<UIText> m_EndingLine1;  // "THANK YOU MARIO!"
    std::shared_ptr<UIText> m_EndingLine2;  // "YOUR QUEST IS OVER."
    bool m_AxeEndingInitialized = false;

    // Game over screen elements
    std::shared_ptr<UIText> m_FinalScoreText;
    std::shared_ptr<UIImage> m_MarioPreview;
    std::string m_CurrentPreviewSpritePath;
};

}  // namespace Mario

#endif  // MARIO_UI_MANAGER_HPP
