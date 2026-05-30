/**
 * @file UIManager.cpp
 * @brief Implementation of UIManager and all IUIPanel subclasses.
 *        CoinUI and FloatingText are each implemented in their own .cpp files.
 * @inheritance None <- UIManager      (dispatcher / service)
 *              IUIPanel <- HUDPanel
 *              IUIPanel <- TitlePanel
 *              IUIPanel <- LoadingPanel
 *              IUIPanel <- SimpleTextPanel
 *              IUIPanel <- ESCMenuPanel
 *              IUIPanel <- AxeEndingPanel
 */
#include "Mario/UI/UIManager.hpp"

#include <cstdio>
#include <fstream>

#include "Mario/Core/GameConfig.hpp"
#include "Util/Color.hpp"
#include "Util/Logger.hpp"
#include "config.hpp"

namespace Mario {

// ============================================================================
// HUDPanel
// ============================================================================

HUDPanel::HUDPanel(const std::string& fontPath, int fontSize)
    : m_FontPath(fontPath), m_FontSize(fontSize) {
    auto white = Util::Color::FromRGB(255, 255, 255);
    m_HeaderMario =
        std::make_shared<UIText>(fontPath, fontSize, "MARIO", white);
    m_HeaderWorld =
        std::make_shared<UIText>(fontPath, fontSize, "WORLD", white);
    m_HeaderTime = std::make_shared<UIText>(fontPath, fontSize, "TIME", white);
    m_ScoreText = std::make_shared<UIText>(fontPath, fontSize, "000000", white);
    m_WorldText = std::make_shared<UIText>(fontPath, fontSize, "1-1", white);
    m_TimeText = std::make_shared<UIText>(fontPath, fontSize, "400", white);
    m_CoinUI =
        std::make_shared<CoinUI>(fontPath, fontSize, 420.0f, 32.0f, 2.0f);
}

void HUDPanel::Register(Util::Renderer& renderer) {
    renderer.AddChild(m_HeaderMario);
    renderer.AddChild(m_HeaderWorld);
    renderer.AddChild(m_HeaderTime);
    renderer.AddChild(m_ScoreText);
    renderer.AddChild(m_WorldText);
    renderer.AddChild(m_TimeText);
    renderer.AddChild(m_CoinUI->GetCoinImage());
    renderer.AddChild(m_CoinUI->GetCountText());
}

void HUDPanel::Show() {
    m_HeaderMario->SetVisible(true);
    m_HeaderWorld->SetVisible(true);
    m_HeaderTime->SetVisible(true);
    m_ScoreText->SetVisible(true);
    m_WorldText->SetVisible(true);
    m_TimeText->SetVisible(true);
    if (m_CoinUI) {
        m_CoinUI->GetCoinImage()->SetVisible(true);
        m_CoinUI->GetCountText()->SetVisible(true);
    }
}

void HUDPanel::Hide() {
    m_HeaderMario->SetVisible(false);
    m_HeaderWorld->SetVisible(false);
    m_HeaderTime->SetVisible(false);
    m_ScoreText->SetVisible(false);
    m_WorldText->SetVisible(false);
    m_TimeText->SetVisible(false);
    if (m_CoinUI) {
        m_CoinUI->GetCoinImage()->SetVisible(false);
        m_CoinUI->GetCountText()->SetVisible(false);
    }
}

void HUDPanel::Refresh(const GameStateManager& gs) {
    // HUD layout for 1280x720 PTSD window
    // PTSD coordinate system: center (0,0), left=-640, right=640,
    // top=360, bottom=-360

    // --- MARIO label and score (far left) ---
    float marioHeaderX = 140.0f;
    float marioHeaderY = 16.0f;
    float marioScoreY = 32.0f;
    m_HeaderMario->SetPosition(GameConfig::ScreenXToPTSD(marioHeaderX),
                               GameConfig::ScreenYToPTSD(marioHeaderY));
    m_ScoreText->SetPosition(GameConfig::ScreenXToPTSD(marioHeaderX),
                             GameConfig::ScreenYToPTSD(marioScoreY));
    char scoreStr[10];
    snprintf(scoreStr, sizeof(scoreStr), "%06d", gs.GetScore());
    m_ScoreText->SetTextContent(scoreStr);

    // --- Coins (left-center) with animated icon ---
    if (m_CoinUI) {
        m_CoinUI->Update(gs.GetCoins());
    }

    // --- WORLD label and level (center) ---
    float worldHeaderY = 16.0f;
    float worldLevelY = 32.0f;
    m_HeaderWorld->SetPosition(
        GameConfig::ScreenXToPTSD(GameConfig::WINDOW_WIDTH / 2.0f),
        GameConfig::ScreenYToPTSD(worldHeaderY));
    m_WorldText->SetPosition(
        GameConfig::ScreenXToPTSD(GameConfig::WINDOW_WIDTH / 2.0f),
        GameConfig::ScreenYToPTSD(worldLevelY));
    m_WorldText->SetTextContent(gs.GetLevelName());

    // --- TIME label and value (far right) ---
    float timeHeaderX = 1100.0f;
    float timeHeaderY = 16.0f;
    float timeValueX = 1115.0f;
    float timeValueY = 32.0f;
    m_HeaderTime->SetPosition(GameConfig::ScreenXToPTSD(timeHeaderX),
                              GameConfig::ScreenYToPTSD(timeHeaderY));
    m_TimeText->SetPosition(GameConfig::ScreenXToPTSD(timeValueX),
                            GameConfig::ScreenYToPTSD(timeValueY));
    char timeStr[10];
    snprintf(timeStr, sizeof(timeStr), "%03d", gs.GetTimeRemaining());
    m_TimeText->SetTextContent(timeStr);

    // --- Time warning flash (< 100 seconds) ---
    int timeRemaining = gs.GetTimeRemaining();
    if (timeRemaining < 100 && timeRemaining > 0) {
        int flashFrame = (m_FlashCounter / 8) % 2;
        auto color = (flashFrame == 0) ? Util::Color::FromRGB(255, 0, 0)
                                       : Util::Color::FromRGB(255, 255, 255);
        m_TimeText->SetTextColor(color);
        m_HeaderTime->SetTextColor(color);
        m_FlashCounter++;
    } else {
        m_FlashCounter = 0;
        m_TimeText->SetTextColor(Util::Color::FromRGB(255, 255, 255));
        m_HeaderTime->SetTextColor(Util::Color::FromRGB(255, 255, 255));
    }
}

// ============================================================================
// TitlePanel
// ============================================================================

TitlePanel::TitlePanel(const std::string& fontPath, int fontSize) {
    auto white = Util::Color::FromRGB(255, 255, 255);
    m_TitleLabel = std::make_shared<UIText>(fontPath, fontSize * 2, "", white);
    m_SubLabel = std::make_shared<UIText>(fontPath, fontSize, "", white);
    m_TitleLabel->SetVisible(false);
    m_SubLabel->SetVisible(false);
}

void TitlePanel::Register(Util::Renderer& renderer) {
    renderer.AddChild(m_TitleLabel);
    renderer.AddChild(m_SubLabel);
}

void TitlePanel::Show() {
    m_TitleLabel->SetVisible(true);
    m_SubLabel->SetVisible(true);
}

void TitlePanel::Hide() {
    m_TitleLabel->SetVisible(false);
    m_SubLabel->SetVisible(false);
}

void TitlePanel::Refresh([[maybe_unused]] const GameStateManager& gs) {
    m_TitleLabel->SetTextContent("SUPER MARIO BROS");
    m_TitleLabel->SetPosition(0.0f, 100.0f);
    m_SubLabel->SetTextContent("PRESS ENTER TO START");
    m_SubLabel->SetPosition(0.0f, -50.0f);
}

// ============================================================================
// LoadingPanel
// ============================================================================

LoadingPanel::LoadingPanel(const std::string& fontPath, int fontSize) {
    auto white = Util::Color::FromRGB(255, 255, 255);
    m_WorldLabel = std::make_shared<UIText>(fontPath, fontSize * 2, "", white);
    m_LivesText = std::make_shared<UIText>(fontPath, fontSize, "", white);

    // Pre-load the Mario preview sprite so the OpenGL texture is uploaded
    // before the first loading screen frame is drawn (avoids one-frame blank).
    std::string marioSpritePath =
        std::string(RESOURCE_DIR) + "/Sprites/MarioIdle.png";
    m_MarioPreview = std::make_shared<UIImage>(marioSpritePath);
    m_MarioPreview->SetPosition(-30.0f, -10.0f);
    m_MarioPreview->m_Transform.scale = {GameConfig::DRAW_SCALE, GameConfig::DRAW_SCALE};
    m_MarioPreview->SetZIndex(100.0f);  // Set ZIndex to 100.0f to match UIText and be on the exact same UI layer

    m_WorldLabel->SetVisible(false);
    m_LivesText->SetVisible(false);
    m_MarioPreview->SetVisible(false);
}

void LoadingPanel::Register(Util::Renderer& renderer) {
    renderer.AddChild(m_WorldLabel);
    renderer.AddChild(m_LivesText);
    renderer.AddChild(m_MarioPreview);
}

void LoadingPanel::Show() {
    m_WorldLabel->SetVisible(true);
    m_LivesText->SetVisible(true);
    m_MarioPreview->SetVisible(true);
}

void LoadingPanel::Hide() {
    m_WorldLabel->SetVisible(false);
    m_LivesText->SetVisible(false);
    m_MarioPreview->SetVisible(false);
}

void LoadingPanel::Refresh(const GameStateManager& gs) {
    m_WorldLabel->SetTextContent("WORLD " + gs.GetLevelName());
    m_WorldLabel->SetPosition(0.0f, 50.0f);

    int lives = gs.GetLives();
    std::string livesStr =
        "x " + std::string(lives < 10 ? "0" : "") + std::to_string(lives);
    m_LivesText->SetTextContent(livesStr);
    m_LivesText->SetPosition(30.0f, -10.0f);

    // Ensure the Mario preview is placed exactly inside the blue box on the left of "x 03"
    m_MarioPreview->SetPosition(-30.0f, -10.0f);
    m_MarioPreview->m_Transform.scale = {GameConfig::DRAW_SCALE, GameConfig::DRAW_SCALE};
}

// ============================================================================
// SimpleTextPanel (GameOver + GameWon)
// ============================================================================

SimpleTextPanel::SimpleTextPanel(const std::string& fontPath, int fontSize,
                                 const std::string& titleText)
    : m_TitleText(titleText) {
    auto white = Util::Color::FromRGB(255, 255, 255);
    m_TitleLabel =
        std::make_shared<UIText>(fontPath, fontSize * 2, titleText, white);
    m_ScoreText = std::make_shared<UIText>(fontPath, fontSize, "", white);
    m_TitleLabel->SetVisible(false);
    m_ScoreText->SetVisible(false);
}

void SimpleTextPanel::Register(Util::Renderer& renderer) {
    renderer.AddChild(m_TitleLabel);
    renderer.AddChild(m_ScoreText);
}

void SimpleTextPanel::Show() {
    m_TitleLabel->SetVisible(true);
    m_ScoreText->SetVisible(true);
}

void SimpleTextPanel::Hide() {
    m_TitleLabel->SetVisible(false);
    m_ScoreText->SetVisible(false);
}

void SimpleTextPanel::Refresh(const GameStateManager& gs) {
    m_TitleLabel->SetTextContent(m_TitleText);
    m_TitleLabel->SetPosition(0.0f, 100.0f);

    char scoreStr[50];
    snprintf(scoreStr, sizeof(scoreStr), "FINAL SCORE: %06d", gs.GetScore());
    m_ScoreText->SetTextContent(scoreStr);
    m_ScoreText->SetPosition(0.0f, -50.0f);
}

// ============================================================================
// ESCMenuPanel
// ============================================================================

ESCMenuPanel::ESCMenuPanel(const std::string& fontPath, int fontSize) {
    auto white = Util::Color::FromRGB(255, 255, 255);
    m_PausedLabel =
        std::make_shared<UIText>(fontPath, fontSize * 2, "PAUSED", white);
    m_PausedLabel->SetVisible(false);

    std::vector<std::string> options = {"RESUME", "1-1", "1-2", "8-4",
                                        "POWER: SMALL", "CHEAT: OFF"};
    for (const auto& opt : options) {
        auto text = std::make_shared<UIText>(fontPath, fontSize, opt, white);
        text->SetVisible(false);
        m_MenuTexts.push_back(text);
    }
}

void ESCMenuPanel::SetMenuContext(int selection,
                                  const std::string& powerStateName) {
    m_Selection = selection;
    m_PowerStateName = powerStateName;
}

void ESCMenuPanel::Register(Util::Renderer& renderer) {
    renderer.AddChild(m_PausedLabel);
    for (auto& t : m_MenuTexts) {
        renderer.AddChild(t);
    }
}

void ESCMenuPanel::Show() {
    m_PausedLabel->SetVisible(true);
    for (auto& t : m_MenuTexts) {
        t->SetVisible(true);
    }
}

void ESCMenuPanel::Hide() {
    m_PausedLabel->SetVisible(false);
    for (auto& t : m_MenuTexts) {
        t->SetVisible(false);
    }
}

void ESCMenuPanel::Refresh(const GameStateManager& gs) {
    m_PausedLabel->SetPosition(0.0f, 280.0f);

    // Update the POWER cheat item text to reflect the current state.
    if (m_MenuTexts.size() >= 6) {
        m_MenuTexts[4]->SetTextContent("POWER: " + m_PowerStateName);
        m_MenuTexts[5]->SetTextContent(std::string("CHEAT: ") + (gs.IsCheatModeActive() ? "ON" : "OFF"));
    }

    float startY = 120.0f;
    for (size_t i = 0; i < m_MenuTexts.size(); ++i) {
        m_MenuTexts[i]->SetPosition(0.0f,
                                    startY - static_cast<float>(i) * 55.0f);
        auto color = (static_cast<int>(i) == m_Selection)
                         ? Util::Color::FromRGB(255, 0, 0)       // highlighted
                         : Util::Color::FromRGB(255, 255, 255);  // normal
        m_MenuTexts[i]->SetTextColor(color);
    }
}

// ============================================================================
// AxeEndingPanel
// ============================================================================

AxeEndingPanel::AxeEndingPanel(const std::string& fontPath, int fontSize) {
    auto yellow = Util::Color::FromRGB(255, 215, 0);
    auto white = Util::Color::FromRGB(255, 255, 255);
    m_Line1 = std::make_shared<UIText>(fontPath, fontSize * 2,
                                       "THANK YOU MARIO!", yellow);
    m_Line2 = std::make_shared<UIText>(fontPath, fontSize,
                                       "YOUR QUEST IS OVER.", white);
    m_Line1->SetVisible(false);
    m_Line2->SetVisible(false);
}

void AxeEndingPanel::Register(Util::Renderer& renderer) {
    renderer.AddChild(m_Line1);
    renderer.AddChild(m_Line2);
}

void AxeEndingPanel::Show() {
    m_Line1->SetVisible(true);
    m_Line2->SetVisible(true);
}

void AxeEndingPanel::Hide() {
    m_Line1->SetVisible(false);
    m_Line2->SetVisible(false);
}

void AxeEndingPanel::Refresh([[maybe_unused]] const GameStateManager& gs) {
    m_Line1->SetVisible(m_ShowCredits);
    m_Line2->SetVisible(m_ShowCredits);
    if (m_ShowCredits) {
        m_Line1->SetPosition(0.0f, 60.0f);
        m_Line2->SetPosition(0.0f, -20.0f);
    }
}

// ============================================================================
// UIManager
// ============================================================================

UIManager::UIManager(GameStateManager* gameState)
    : m_GameState(gameState),
      m_HUDPanel(m_FontPath, m_FontSize),
      m_TitlePanel(m_FontPath, m_FontSize),
      m_LoadingPanel(m_FontPath, m_FontSize),
      m_GameOverPanel(m_FontPath, m_FontSize, "GAME OVER"),
      m_GameWonPanel(m_FontPath, m_FontSize, "WORLD CLEARED"),
      m_ESCMenuPanel(m_FontPath, m_FontSize),
      m_AxeEndingPanel(m_FontPath, m_FontSize) {
    auto white = Util::Color::FromRGB(255, 255, 255);

    // Register all panels into the shared renderer.
    m_HUDPanel.Register(m_UIRenderer);
    m_TitlePanel.Register(m_UIRenderer);
    m_LoadingPanel.Register(m_UIRenderer);
    m_GameOverPanel.Register(m_UIRenderer);
    m_GameWonPanel.Register(m_UIRenderer);
    m_ESCMenuPanel.Register(m_UIRenderer);
    m_AxeEndingPanel.Register(m_UIRenderer);

    // Build panel dispatch map (HUD is handled separately as it overlays
    // gameplay states; only scene-specific panels are in the map).
    m_PanelMap = {
        {State::TITLE, &m_TitlePanel},
        {State::LOADING, &m_LoadingPanel},
        {State::GAME_OVER, &m_GameOverPanel},
        {State::GAME_WON, &m_GameWonPanel},
        {State::ESC_MENU, &m_ESCMenuPanel},
        {State::AXE_SEQUENCE, &m_AxeEndingPanel},
    };

    // FPS counter (bottom-right corner).
    m_FPSText =
        std::make_shared<UIText>(m_FontPath, m_FontSize, "FPS: --", white);
    m_FPSText->SetPosition(520.0f, -340.0f);
    m_UIRenderer.AddChild(m_FPSText);

    // Copyright text (bottom-left corner); prefer a Chinese-capable font.
    std::string chineseFontPath = m_FontPath;
    {
        std::ifstream f1("C:/Windows/Fonts/msjh.ttc");
        if (f1.good()) {
            chineseFontPath = "C:/Windows/Fonts/msjh.ttc";
        } else {
            std::ifstream f2("C:/Windows/Fonts/msjh.ttf");
            if (f2.good()) {
                chineseFontPath = "C:/Windows/Fonts/msjh.ttf";
            }
        }
    }
    m_CopyrightText = std::make_shared<UIText>(
        chineseFontPath, m_FontSize, "113820033 電資二 謝奕宏", white);
    m_CopyrightText->SetPosition(-620.0f, -340.0f);
    m_UIRenderer.AddChild(m_CopyrightText);

    // Construct gold-colored Cheat Mode text centered at the bottom of the screen
    m_CheatModeText = std::make_shared<UIText>(
        m_FontPath, m_FontSize, "CHEAT MODE ENABLED", Util::Color::FromRGB(255, 215, 0));
    m_CheatModeText->SetPosition(0.0f, -340.0f);
    m_CheatModeText->SetVisible(false);
    m_UIRenderer.AddChild(m_CheatModeText);
}

void UIManager::HideAllScenePanels() {
    for (auto& [state, panel] : m_PanelMap) {
        panel->Hide();
    }
}

void UIManager::Update(State currentState, int escMenuSelection,
                       const std::string& powerStateName) {
    if (!m_GameState) return;

    // --- FPS counter ---
    m_FrameCount++;
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                       now - m_LastFPSTime)
                       .count();
    if (elapsed >= 1000) {
        m_FPS = static_cast<int>(m_FrameCount * 1000.0f / elapsed);
        m_FrameCount = 0;
        m_LastFPSTime = now;
        char buf[32];
        snprintf(buf, sizeof(buf), "FPS: %d", m_FPS);
        m_FPSText->SetTextContent(buf);
    }

    // --- HUD visibility (shown during all active gameplay states) ---
    bool showHUD =
        (currentState == State::PLAYING || currentState == State::ESC_MENU);
    if (showHUD) {
        m_HUDPanel.Show();
        m_HUDPanel.Refresh(*m_GameState);
    } else {
        m_HUDPanel.Hide();
    }

    // --- Cheat Mode bottom UI visibility ---
    if (m_GameState && m_GameState->IsCheatModeActive() && showHUD) {
        m_CheatModeText->SetVisible(true);
    } else {
        m_CheatModeText->SetVisible(false);
    }

    // --- Scene panel dispatch ---
    HideAllScenePanels();
    auto it = m_PanelMap.find(currentState);
    if (it != m_PanelMap.end()) {
        IUIPanel* panel = it->second;

        // Supply extra context to panels that need it before Refresh().
        if (currentState == State::ESC_MENU) {
            m_ESCMenuPanel.SetMenuContext(escMenuSelection, powerStateName);
        }
        if (currentState == State::AXE_SEQUENCE) {
            m_AxeEndingPanel.SetShowCredits(m_EndingTextPhase ==
                                            EndingTextPhase::CREDITS);
        }

        panel->Show();
        panel->Refresh(*m_GameState);
    }

    // --- Floating text (score pop-ups, 1UP, etc.) ---
    if (!m_FloatingTexts.empty()) {
        auto it2 = m_FloatingTexts.begin();
        while (it2 != m_FloatingTexts.end()) {
            (*it2)->Update();
            if ((*it2)->IsExpired()) {
                m_UIRenderer.RemoveChild((*it2)->GetUIText());
                it2 = m_FloatingTexts.erase(it2);
            } else {
                ++it2;
            }
        }
    }

    m_UIRenderer.Update();
}

void UIManager::AddFloatingText(float screenX, float screenY,
                                const std::string& text, int frames) {
    auto ft = std::make_shared<FloatingText>(screenX, screenY, text, frames);
    m_FloatingTexts.push_back(ft);
    m_UIRenderer.AddChild(ft->GetUIText());
    LOG_DEBUG("Added floating text: '{}' at screen ({}, {})", text, screenX,
              screenY);
}

}  // namespace Mario
