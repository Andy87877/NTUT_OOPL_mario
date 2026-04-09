/**
 * @file UIManager.cpp
 * @brief Implementation of UI manager with full text rendering logic.
 * @inheritance None (Service)
 */
#include "Mario/UIManager.hpp"

#include <cstdio>

#include "Util/Logger.hpp"

namespace Mario {

UIManager::UIManager(GameStateManager* gameState) : m_GameState(gameState) {
    auto colorWhite = Util::Color::FromRGB(255, 255, 255);

    // Center labels for scenes
    m_CenterLabel =
        std::make_shared<UIText>(m_FontPath, m_FontSize * 2, "", colorWhite);
    m_SubLabel =
        std::make_shared<UIText>(m_FontPath, m_FontSize, "", colorWhite);

    m_UIRenderer.AddChild(m_CenterLabel);
    m_UIRenderer.AddChild(m_SubLabel);

    InitHUD();
    InitESCMenu();
}

void UIManager::InitHUD() {
    auto colorWhite = Util::Color::FromRGB(255, 255, 255);

    // --- Initialize HUD Text Elements ---
    m_HeaderMario =
        std::make_shared<UIText>(m_FontPath, m_FontSize, "MARIO", colorWhite);
    m_HeaderWorld =
        std::make_shared<UIText>(m_FontPath, m_FontSize, "WORLD", colorWhite);
    m_HeaderTime =
        std::make_shared<UIText>(m_FontPath, m_FontSize, "TIME", colorWhite);

    // Values matching C# format exactly
    m_ScoreText =
        std::make_shared<UIText>(m_FontPath, m_FontSize, "000000", colorWhite);
    m_CoinsText = std::make_shared<UIText>(m_FontPath, m_FontSize, "x00",
                                           colorWhite);  // No space!
    m_WorldText =
        std::make_shared<UIText>(m_FontPath, m_FontSize, "1-1", colorWhite);
    m_TimeText =
        std::make_shared<UIText>(m_FontPath, m_FontSize, "400", colorWhite);

    m_UIRenderer.AddChild(m_HeaderMario);
    m_UIRenderer.AddChild(m_HeaderWorld);
    m_UIRenderer.AddChild(m_HeaderTime);
    m_UIRenderer.AddChild(m_ScoreText);
    m_UIRenderer.AddChild(m_CoinsText);
    m_UIRenderer.AddChild(m_WorldText);
    m_UIRenderer.AddChild(m_TimeText);
}

void UIManager::InitESCMenu() {
    std::vector<std::string> menuOptions = {"RESUME", "1-1", "1-2", "8-4"};
    for (const auto& opt : menuOptions) {
        auto text = std::make_shared<UIText>(
            m_FontPath, m_FontSize, opt, Util::Color::FromRGB(255, 255, 255));
        text->SetVisible(false);
        m_MenuTexts.push_back(text);
        m_UIRenderer.AddChild(text);
    }
}

void UIManager::Update(State currentState, int escMenuSelection) {
    if (!m_GameState) return;

    // Hide everything first
    m_CenterLabel->SetVisible(false);
    m_SubLabel->SetVisible(false);

    bool showHUD =
        (currentState == State::PLAYING || currentState == State::ESC_MENU);
    m_HeaderMario->SetVisible(showHUD);
    m_HeaderWorld->SetVisible(showHUD);
    m_HeaderTime->SetVisible(showHUD);
    m_ScoreText->SetVisible(showHUD);
    m_CoinsText->SetVisible(showHUD);
    m_WorldText->SetVisible(showHUD);
    m_TimeText->SetVisible(showHUD);

    for (auto& menuText : m_MenuTexts) {
        menuText->SetVisible(false);
    }

    if (showHUD) {
        UpdateHUD();
    }

    switch (currentState) {
        case State::TITLE:
            UpdateTitleScreen();
            break;
        case State::LOADING:
            UpdateLoadingScreen();
            break;
        case State::GAME_OVER:
            UpdateGameOverScreen();
            break;
        case State::ESC_MENU:
            UpdateESCMenu(escMenuSelection);
            break;
        default:
            break;
    }

    // Floating text handles its own logic, leaving it out of renderer for now,
    // or just logic
    if (!m_FloatingTexts.empty()) {
        auto it = m_FloatingTexts.begin();
        while (it != m_FloatingTexts.end()) {
            (*it)->Update();
            if ((*it)->IsExpired()) {
                m_UIRenderer.RemoveChild((*it)->GetUIText());
                it = m_FloatingTexts.erase(it);
            } else {
                ++it;
            }
        }
    }

    // Render UI!
    m_UIRenderer.Update();
}

void UIManager::UpdateHUD() {
    // HUD layout for 1280x720 PTSD window
    // Window dimensions: 1280x720
    // PTSD coordinate system: center (0,0), left=-640, right=640, top=360,
    // bottom=-360

    // Screen coordinates in pixels: (0-1280, 0-720)
    // Conversion to PTSD: ptsd_x = screen_x - 640, ptsd_y = 360 - screen_y

    // --- MARIO Label & Score (Far Left) ---
    float marioHeaderX = 140.0f;  // shifted right
    float marioHeaderY = 16.0f;   // pixels from top
    float marioScoreY = 32.0f;

    m_HeaderMario->SetPosition(marioHeaderX - 640.0f, 360.0f - marioHeaderY);
    m_ScoreText->SetPosition(marioHeaderX - 640.0f, 360.0f - marioScoreY);

    char scoreStr[10];
    snprintf(scoreStr, sizeof(scoreStr), "%06d", m_GameState->GetScore());
    m_ScoreText->SetTextContent(scoreStr);

    // --- COINS (Left-Center) ---
    float coinsX = 480.0f;  // shifted right
    float coinsY = 32.0f;

    m_CoinsText->SetPosition(coinsX - 640.0f, 360.0f - coinsY);
    char coinStr[10];
    snprintf(coinStr, sizeof(coinStr), "x%02d", m_GameState->GetCoins());
    m_CoinsText->SetTextContent(coinStr);

    // --- WORLD Label & Level (Center) ---
    // Center horizontally (0.0f), aligned with other HUD headers at top
    float worldHeaderY = 16.0f;  // Same level as MARIO and TIME headers
    float worldLevelY = 32.0f;   // Same level as score and time values

    m_HeaderWorld->SetPosition(0.0f, 360.0f - worldHeaderY);
    m_WorldText->SetPosition(0.0f, 360.0f - worldLevelY);
    m_WorldText->SetTextContent(m_GameState->GetLevelName());

    // --- TIME Label & Value (Far Right) ---
    float timeHeaderX = 1100.0f;  // pixels from left
    float timeHeaderY = 16.0f;
    float timeValueX = 1115.0f;
    float timeValueY = 32.0f;

    m_HeaderTime->SetPosition(timeHeaderX - 640.0f, 360.0f - timeHeaderY);
    m_TimeText->SetPosition(timeValueX - 640.0f, 360.0f - timeValueY);

    char timeStr[10];
    snprintf(timeStr, sizeof(timeStr), "%03d", m_GameState->GetTimeRemaining());
    m_TimeText->SetTextContent(timeStr);
}

void UIManager::UpdateTitleScreen() {
    m_CenterLabel->SetVisible(true);
    m_CenterLabel->SetTextContent("SUPER MARIO BROS");
    m_CenterLabel->SetPosition(0.0f,
                               100.0f);  // Center horizontally, upper area

    m_SubLabel->SetVisible(true);
    m_SubLabel->SetTextContent("PRESS ENTER TO START");
    m_SubLabel->SetPosition(0.0f, -50.0f);  // Center horizontally, lower area
}

void UIManager::UpdateLoadingScreen() {
    m_CenterLabel->SetVisible(true);
    m_CenterLabel->SetTextContent("WORLD " + m_GameState->GetLevelName());
    m_CenterLabel->SetPosition(0.0f, 50.0f);  // Centered horizontally

    m_SubLabel->SetVisible(true);
    m_SubLabel->SetTextContent("x  " + std::to_string(m_GameState->GetLives()));
    m_SubLabel->SetPosition(0.0f,
                            -50.0f);  // Below the WORLD label, also centered
}

void UIManager::UpdateGameOverScreen() {
    m_CenterLabel->SetVisible(true);
    m_CenterLabel->SetTextContent("GAME OVER");
    m_CenterLabel->SetPosition(0.0f, 0.0f);  // Centered on screen
}

void UIManager::UpdateESCMenu(int selection) {
    m_SubLabel->SetVisible(true);
    m_SubLabel->SetTextContent("PAUSED");
    m_SubLabel->SetPosition(-60.0f, 200.0f);  // Upper center area

    float startY = 100.0f;
    for (size_t i = 0; i < m_MenuTexts.size(); ++i) {
        m_MenuTexts[i]->SetVisible(true);
        m_MenuTexts[i]->SetPosition(
            0.0f, startY - (i * 60.0f));  // Centered, spread downward

        if ((int)i == selection) {
            m_MenuTexts[i]->SetTextColor(
                Util::Color::FromRGB(255, 0, 0));  // Highlight Selection (RED)
        } else {
            m_MenuTexts[i]->SetTextColor(
                Util::Color::FromRGB(255, 255, 255));  // White
        }
    }
}

void UIManager::AddFloatingText(float worldX, float worldY,
                                const std::string& text, int frames) {
    auto floatingText =
        std::make_shared<FloatingText>(worldX, worldY, text, frames);
    m_FloatingTexts.push_back(floatingText);
    m_UIRenderer.AddChild(floatingText->GetUIText());
    LOG_DEBUG("Added floating text: '{}' at ({}, {})", text, worldX, worldY);
}

}  // namespace Mario
