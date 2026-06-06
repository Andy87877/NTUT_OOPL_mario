/**
 * @file HUDPanel.cpp
 * @brief HUD panel implementation displaying game metrics.
 * @inheritance IUIPanel <- HUDPanel
 */
#include "Mario/UI/HUDPanel.hpp"

#include <cstdio>

#include "Mario/Core/GameConfig.hpp"
#include "Util/Color.hpp"

namespace Mario {

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
    float timeHeaderX = 1140.0f;
    float timeHeaderY = 16.0f;
    float timeValueX = 1155.0f;
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

}  // namespace Mario
