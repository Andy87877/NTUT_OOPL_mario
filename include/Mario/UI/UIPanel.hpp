/**
 * @file UIPanel.hpp
 * @brief IUIPanel Strategy interface and all concrete per-screen panel classes.
 *        Each IUIPanel owns the widgets for exactly one game screen and
 *        implements Show/Hide/Refresh so UIManager can dispatch via a panel
 *        map instead of a monolithic switch-case.
 *
 *        To add a new screen:
 *          1. Subclass IUIPanel.
 *          2. Construct it in UIManager's member list.
 *          3. Register it in UIManager's m_PanelMap.
 *          Zero changes to UIManager::Update().
 *
 * @inheritance IUIPanel (interface root)
 *              IUIPanel <- HUDPanel
 *              IUIPanel <- TitlePanel
 *              IUIPanel <- LoadingPanel
 *              IUIPanel <- SimpleTextPanel  (GameOver / GameWon)
 *              IUIPanel <- ESCMenuPanel
 *              IUIPanel <- AxeEndingPanel
 */
#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Mario/UI/CoinUI.hpp"
#include "Mario/Level/GameStateManager.hpp"
#include "Mario/UI/UIWidgets.hpp"
#include "Util/Color.hpp"
#include "Util/Renderer.hpp"
#include "config.hpp"

namespace Mario {

// ============================================================================
// IUIPanel — Strategy interface
// ============================================================================

/**
 * Interface for a single game-screen's UI panel.
 * Each concrete panel owns its UIText/UIImage widgets and is responsible
 * for registering them into the renderer, showing/hiding them, and refreshing
 * their content from the game state.
 */
class IUIPanel {
   public:
    virtual ~IUIPanel() = default;

    /** Register all child widgets into the renderer once at startup. */
    virtual void Register(Util::Renderer& renderer) = 0;

    /** Make all owned widgets visible. */
    virtual void Show() = 0;

    /** Hide all owned widgets. */
    virtual void Hide() = 0;

    /** Refresh text content and layout from the current game state. */
    virtual void Refresh(const GameStateManager& gs) = 0;
};

// ============================================================================
// HUDPanel — score / world / time / coins (visible during all gameplay states)
// ============================================================================

class HUDPanel : public IUIPanel {
   public:
    HUDPanel(const std::string& fontPath, int fontSize);

    void Register(Util::Renderer& renderer) override;
    void Show() override;
    void Hide() override;
    void Refresh(const GameStateManager& gs) override;

   private:
    std::string m_FontPath;
    int m_FontSize;

    std::shared_ptr<UIText> m_HeaderMario;
    std::shared_ptr<UIText> m_HeaderWorld;
    std::shared_ptr<UIText> m_HeaderTime;
    std::shared_ptr<UIText> m_ScoreText;
    std::shared_ptr<UIText> m_WorldText;
    std::shared_ptr<UIText> m_TimeText;
    std::shared_ptr<CoinUI> m_CoinUI;

    int m_FlashCounter = 0;
};

// ============================================================================
// TitlePanel — "SUPER MARIO BROS" + "PRESS ENTER TO START"
// ============================================================================

class TitlePanel : public IUIPanel {
   public:
    TitlePanel(const std::string& fontPath, int fontSize);

    void Register(Util::Renderer& renderer) override;
    void Show() override;
    void Hide() override;
    void Refresh(const GameStateManager& gs) override;

   private:
    std::shared_ptr<UIText> m_TitleLabel;
    std::shared_ptr<UIText> m_SubLabel;
};

// ============================================================================
// LoadingPanel — "WORLD X-X" + lives count + Mario sprite
// ============================================================================

class LoadingPanel : public IUIPanel {
   public:
    LoadingPanel(const std::string& fontPath, int fontSize);

    void Register(Util::Renderer& renderer) override;
    void Show() override;
    void Hide() override;
    void Refresh(const GameStateManager& gs) override;

   private:
    std::shared_ptr<UIText> m_WorldLabel;
    std::shared_ptr<UIText> m_LivesText;
    std::shared_ptr<UIImage> m_MarioPreview;
};

// ============================================================================
// SimpleTextPanel — reusable for GameOver ("GAME OVER") and GameWon
//                   ("WORLD CLEARED"). Shows a title + final score.
// ============================================================================

class SimpleTextPanel : public IUIPanel {
   public:
    /**
     * @param titleText  Static title displayed at the top (e.g. "GAME OVER").
     */
    SimpleTextPanel(const std::string& fontPath, int fontSize,
                    const std::string& titleText);

    void Register(Util::Renderer& renderer) override;
    void Show() override;
    void Hide() override;
    void Refresh(const GameStateManager& gs) override;

   private:
    std::string m_TitleText;
    std::shared_ptr<UIText> m_TitleLabel;
    std::shared_ptr<UIText> m_ScoreText;
};

// ============================================================================
// ESCMenuPanel — pause menu with selection highlight
// ============================================================================

class ESCMenuPanel : public IUIPanel {
   public:
    ESCMenuPanel(const std::string& fontPath, int fontSize);

    /**
     * Supply menu-specific context before calling Refresh().
     * @param selection      0-based index of the currently highlighted item.
     * @param powerStateName Current cheat power name ("SMALL"/"BIG"/etc.).
     */
    void SetMenuContext(int selection, const std::string& powerStateName);

    void Register(Util::Renderer& renderer) override;
    void Show() override;
    void Hide() override;
    void Refresh(const GameStateManager& gs) override;

   private:
    int m_Selection = 0;
    std::string m_PowerStateName = "SMALL";

    std::shared_ptr<UIText> m_PausedLabel;
    std::vector<std::shared_ptr<UIText>> m_MenuTexts;
};

// ============================================================================
// AxeEndingPanel — "THANK YOU MARIO!" / "YOUR QUEST IS OVER."
// ============================================================================

class AxeEndingPanel : public IUIPanel {
   public:
    AxeEndingPanel(const std::string& fontPath, int fontSize);

    /** Set whether credits text should be visible before calling Refresh(). */
    void SetShowCredits(bool show) { m_ShowCredits = show; }

    void Register(Util::Renderer& renderer) override;
    void Show() override;
    void Hide() override;
    void Refresh(const GameStateManager& gs) override;

   private:
    bool m_ShowCredits = false;
    std::shared_ptr<UIText> m_Line1;
    std::shared_ptr<UIText> m_Line2;
};

}  // namespace Mario
