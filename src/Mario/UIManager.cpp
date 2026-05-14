/**
 * @file UIManager.cpp
 * @brief Implementation of UIManager, CoinUI, and FloatingText.
 *        CoinUI and FloatingText are private sub-components exclusively owned
 *        by UIManager; merging their implementations here keeps the entire
 *        HUD sub-system in one compiled unit.
 * @inheritance None <- UIManager  (Service)
 *              None <- CoinUI     (Composite UI component)
 *              None <- FloatingText (UI effect component)
 */
#include "Mario/UIManager.hpp"

#include <cstdio>

#include "Util/Color.hpp"
#include "Util/Logger.hpp"
#include "config.hpp"

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
    m_WorldText =
        std::make_shared<UIText>(m_FontPath, m_FontSize, "1-1", colorWhite);
    m_TimeText =
        std::make_shared<UIText>(m_FontPath, m_FontSize, "400", colorWhite);

    // Initialize CoinUI at position (420, 32) - left-center of HUD, shifted
    // more left 2x scale, positioned to the left
    m_CoinUI =
        std::make_shared<CoinUI>(m_FontPath, m_FontSize, 420.0f, 32.0f, 2.0f);

    m_UIRenderer.AddChild(m_HeaderMario);
    m_UIRenderer.AddChild(m_HeaderWorld);
    m_UIRenderer.AddChild(m_HeaderTime);
    m_UIRenderer.AddChild(m_ScoreText);
    m_UIRenderer.AddChild(m_WorldText);
    m_UIRenderer.AddChild(m_TimeText);

    // Add CoinUI components
    m_UIRenderer.AddChild(m_CoinUI->GetCoinImage());
    m_UIRenderer.AddChild(m_CoinUI->GetCountText());
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
    if (m_AxeEndingInitialized) {
        m_EndingLine1->SetVisible(false);
        m_EndingLine2->SetVisible(false);
    }

    bool showHUD =
        (currentState == State::PLAYING || currentState == State::ESC_MENU);
    m_HeaderMario->SetVisible(showHUD);
    m_HeaderWorld->SetVisible(showHUD);
    m_HeaderTime->SetVisible(showHUD);
    m_ScoreText->SetVisible(showHUD);
    m_WorldText->SetVisible(showHUD);
    m_TimeText->SetVisible(showHUD);

    // Show/hide CoinUI components
    if (m_CoinUI) {
        m_CoinUI->GetCoinImage()->SetVisible(showHUD);
        m_CoinUI->GetCountText()->SetVisible(showHUD);
    }

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
            // Only show UI, do NOT load level yet
            // UpdateLoadingScreen() displays WORLD and lives preview
            UpdateLoadingScreen();
            break;
        case State::GAME_OVER:
            UpdateGameOverScreen();
            break;
        case State::GAME_WON:
            UpdateGameWonScreen();
            break;
        case State::ESC_MENU:
            UpdateESCMenu(escMenuSelection);
            break;
        case State::AXE_SEQUENCE:
            UpdateAxeEndingScreen();
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

    // --- COINS (Left-Center) with animated icon ---
    // CoinUI handles its own positioning and animation
    if (m_CoinUI) {
        m_CoinUI->Update(m_GameState->GetCoins());
    }

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

    // --- TIME WARNING ANIMATION (Flashing when time < 100) ---
    int timeRemaining = m_GameState->GetTimeRemaining();
    if (timeRemaining < 100 && timeRemaining > 0) {
        // Flash effect: alternate between white and red every 8 frames
        int flashFrame = (m_FlashCounter / 8) % 2;
        if (flashFrame == 0) {
            m_TimeText->SetTextColor(Util::Color::FromRGB(255, 0, 0));
            m_HeaderTime->SetTextColor(Util::Color::FromRGB(255, 0, 0));
        } else {
            m_TimeText->SetTextColor(Util::Color::FromRGB(255, 255, 255));
            m_HeaderTime->SetTextColor(Util::Color::FromRGB(255, 255, 255));
        }
        m_FlashCounter++;
    } else {
        m_FlashCounter = 0;
        m_TimeText->SetTextColor(Util::Color::FromRGB(255, 255, 255));
        m_HeaderTime->SetTextColor(Util::Color::FromRGB(255, 255, 255));
    }
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
    m_CenterLabel->SetPosition(0.0f, 50.0f);

    m_SubLabel->SetVisible(true);

    // C# reference format: "x 0N" (e.g. "x 03")
    int lives = m_GameState->GetLives();
    std::string livesStr =
        "x " + std::string(lives < 10 ? "0" : "") + std::to_string(lives);
    m_SubLabel->SetTextContent(livesStr);

    // Position text slightly to the right
    m_SubLabel->SetPosition(10.0f, -10.0f);

    std::string marioSpritePath =
        std::string(RESOURCE_DIR) + "/Sprites/MarioIdle.png";

    if (!m_MarioPreview) {
        m_MarioPreview = std::make_shared<UIImage>(marioSpritePath);
        m_UIRenderer.AddChild(m_MarioPreview);
    }

    m_MarioPreview->SetImagePath(marioSpritePath);
    m_MarioPreview->SetVisible(true);

    // Position Mario left of the text, aligned vertically
    m_MarioPreview->SetPosition(-35.0f, -10.0f);
    m_MarioPreview->m_Transform.scale = {1.0f, 1.0f};
    m_MarioPreview->SetZIndex(101.0f);
}

void UIManager::UpdateGameOverScreen() {
    m_CenterLabel->SetVisible(true);
    m_CenterLabel->SetTextContent("GAME OVER");
    m_CenterLabel->SetPosition(0.0f, 100.0f);  // Centered, upper portion

    m_SubLabel->SetVisible(true);
    char scoreStr[50];
    snprintf(scoreStr, sizeof(scoreStr), "FINAL SCORE: %06d",
             m_GameState->GetScore());
    m_SubLabel->SetTextContent(scoreStr);
    m_SubLabel->SetPosition(0.0f, -50.0f);  // Centered, below GAME OVER

    // Optional: Add "Press ENTER to return to title" hint
    // This would need a third label or dynamic text update
}

void UIManager::UpdateGameWonScreen() {
    m_CenterLabel->SetVisible(true);
    m_CenterLabel->SetTextContent("WORLD CLEARED");
    m_CenterLabel->SetPosition(0.0f, 100.0f);  // Centered, upper portion

    m_SubLabel->SetVisible(true);
    char scoreStr[50];
    snprintf(scoreStr, sizeof(scoreStr), "FINAL SCORE: %06d",
             m_GameState->GetScore());
    m_SubLabel->SetTextContent(scoreStr);
    m_SubLabel->SetPosition(0.0f, -50.0f);  // Centered, below WORLD CLEARED
}

void UIManager::UpdateESCMenu(int selection) {
    m_CenterLabel->SetVisible(true);
    m_CenterLabel->SetTextContent("PAUSED");
    m_CenterLabel->SetPosition(0.0f,
                               280.0f);  // Centered horizontally, above menu

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

void UIManager::InitAxeEndingScreen() {
    auto colorWhite = Util::Color::FromRGB(255, 255, 255);
    auto colorYellow = Util::Color::FromRGB(255, 215, 0);

    m_EndingLine1 = std::make_shared<UIText>(m_FontPath, m_FontSize * 2,
                                             "THANK YOU MARIO!", colorYellow);
    m_EndingLine2 = std::make_shared<UIText>(m_FontPath, m_FontSize,
                                             "YOUR QUEST IS OVER.", colorWhite);

    m_EndingLine1->SetVisible(false);
    m_EndingLine2->SetVisible(false);

    m_UIRenderer.AddChild(m_EndingLine1);
    m_UIRenderer.AddChild(m_EndingLine2);
    m_AxeEndingInitialized = true;
}

void UIManager::UpdateAxeEndingScreen() {
    if (!m_AxeEndingInitialized) {
        InitAxeEndingScreen();
    }

    bool showCredits = (m_EndingTextPhase == EndingTextPhase::CREDITS);
    m_EndingLine1->SetVisible(showCredits);
    m_EndingLine2->SetVisible(showCredits);

    if (showCredits) {
        // Center the "THANK YOU MARIO!" title and subtitle
        m_EndingLine1->SetPosition(0.0f, 60.0f);
        m_EndingLine2->SetPosition(0.0f, -20.0f);
    }
}

void UIManager::AddFloatingText(float screenX, float screenY,
                                const std::string& text, int frames) {
    auto floatingText =
        std::make_shared<FloatingText>(screenX, screenY, text, frames);
    m_FloatingTexts.push_back(floatingText);
    m_UIRenderer.AddChild(floatingText->GetUIText());
    LOG_DEBUG("Added floating text: '{}' at screen ({}, {})", text, screenX,
              screenY);
}

// ============================================================================
// CoinUI — implementation merged here (UIManager is the sole consumer)
// ============================================================================

CoinUI::CoinUI(const std::string& fontPath, int fontSize, float screenX,
               float screenY, float coinScale)
    : m_ScreenX(screenX), m_ScreenY(screenY), m_CoinScale(coinScale) {
    auto colorWhite = Util::Color::FromRGB(255, 255, 255);

    std::string coinPath =
        std::string(RESOURCE_DIR) + COIN_SPRITE_BASE + "1.png";
    m_CoinImage = std::make_shared<UIImage>(coinPath);
    m_CoinImage->m_Transform.scale = {m_CoinScale, m_CoinScale};

    m_CountText =
        std::make_shared<UIText>(fontPath, fontSize, "x00", colorWhite);

    float coinScreenX = screenX - 10.0f;
    m_CoinImage->SetPosition(ScreenXToPTSD(coinScreenX),
                             ScreenYToPTSD(screenY));

    float textOffsetX = coinScreenX + 60.0f;
    m_CountText->SetPosition(ScreenXToPTSD(textOffsetX),
                             ScreenYToPTSD(screenY));
}

void CoinUI::Update(int coinCount) {
    m_AnimationCounter++;
    if (m_AnimationCounter >= FRAME_INTERVAL) {
        m_AnimationCounter = 0;
        m_CurrentFrame = (m_CurrentFrame + 1) % COIN_FRAME_COUNT;
        UpdateCoinSprite();
    }

    char countStr[10];
    snprintf(countStr, sizeof(countStr), "x%02d", coinCount % 100);
    m_CountText->SetTextContent(countStr);
}

float CoinUI::ScreenXToPTSD(float screenX) const { return screenX - 640.0f; }

float CoinUI::ScreenYToPTSD(float screenY) const { return 360.0f - screenY; }

void CoinUI::UpdateCoinSprite() {
    int frameNum = m_CurrentFrame + 1;
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "%s%s%d.png", RESOURCE_DIR,
             COIN_SPRITE_BASE, frameNum);
    m_CoinImage->SetImagePath(buffer);
}

// ============================================================================
// FloatingText — implementation merged here (UIManager is the sole consumer)
// ============================================================================

FloatingText::FloatingText(float ptsdX, float ptsdY, const std::string& text,
                           int durationFrames)
    : m_LifetimeCounter(durationFrames), m_OriginalDuration(durationFrames) {
    m_CurrentPosition = {ptsdX, ptsdY};
    m_UIText =
        std::make_shared<UIText>(std::string(RESOURCE_DIR) + "/Font/mario.ttf",
                                 16, text, Util::Color::FromRGB(255, 255, 255));
    m_UIText->SetPosition(ptsdX, ptsdY);
}

void FloatingText::Update() {
    m_LifetimeCounter--;

    if (m_LifetimeCounter > 0) {
        m_CurrentPosition.y -= 1.0f;
        m_UIText->SetPosition(m_CurrentPosition.x, m_CurrentPosition.y);

        float progress =
            static_cast<float>(m_LifetimeCounter) / m_OriginalDuration;
        int alpha = static_cast<int>(255.0f * progress);
        alpha = (alpha < 0) ? 0 : alpha;
        m_UIText->SetTextColor(
            Util::Color(255, 255, 255, static_cast<Uint8>(alpha)));
    }
}

}  // namespace Mario
