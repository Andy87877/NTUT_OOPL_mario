/**
 * @file GameWonPanel.cpp
 * @brief High-fidelity retro victory screen panel implementation.
 * @inheritance IUIPanel <- GameWonPanel
 */
#include "Mario/UI/GameWonPanel.hpp"

#include <cstdio>

#include "Mario/Core/GameConfig.hpp"
#include "Util/Color.hpp"
#include "config.hpp"

namespace Mario {

GameWonPanel::GameWonPanel(const std::string& fontPath, int fontSize) {
    auto white = Util::Color::FromRGB(255, 255, 255);
    auto gold = Util::Color::FromRGB(255, 215, 0);

    // Load game logo
    std::string logoPath = std::string(RESOURCE_DIR) + "/Sprites/logo.png";
    m_Logo = std::make_shared<UIImage>(logoPath);
    m_Logo->m_Transform.scale = {0.4f, 0.4f};
    m_Logo->SetZIndex(GameConfig::Z_UI);

    // Victory Title
    m_VictoryLabel = std::make_shared<UIText>(fontPath, fontSize * 2, "WORLD CLEARED!", gold);
    m_VictoryLabel->SetZIndex(GameConfig::Z_UI);

    // Mario Sprite Preview (Idle Pose)
    std::string marioPath = std::string(RESOURCE_DIR) + "/Sprites/MarioIdle.png";
    m_MarioImage = std::make_shared<UIImage>(marioPath);
    m_MarioImage->m_Transform.scale = {GameConfig::DRAW_SCALE, GameConfig::DRAW_SCALE};
    m_MarioImage->SetZIndex(GameConfig::Z_UI);

    // Princess Peach Sprite Preview
    std::string princessPath = std::string(RESOURCE_DIR) + "/Sprites/8-4/Princess1.png";
    m_PrincessImage = std::make_shared<UIImage>(princessPath);
    m_PrincessImage->m_Transform.scale = {GameConfig::DRAW_SCALE, GameConfig::DRAW_SCALE};
    // Flip Peach horizontally to face Mario
    m_PrincessImage->m_Transform.scale.x *= -1.0f;
    m_PrincessImage->SetZIndex(GameConfig::Z_UI);

    // Score readout
    m_ScoreLabel = std::make_shared<UIText>(fontPath, fontSize, "", white);
    m_ScoreLabel->SetZIndex(GameConfig::Z_UI);

    // Victory Message
    m_MessageLabel = std::make_shared<UIText>(
        fontPath, fontSize, "THANK YOU MARIO! YOUR QUEST IS OVER.", white);
    m_MessageLabel->SetZIndex(GameConfig::Z_UI);

    // Resolve Traditional Chinese font fallback path for credits
    std::string chineseFontPath = GameConfig::GetChineseFontPath(fontPath);
    m_CreditLabel = std::make_shared<UIText>(
        chineseFontPath, fontSize * 0.8f, "開發者: 113820033 電資二 謝奕宏", white);
    m_CreditLabel->SetZIndex(GameConfig::Z_UI);

    // Return prompt
    m_PromptLabel = std::make_shared<UIText>(
        fontPath, fontSize, "PRESS ENTER TO RETURN TO TITLE", white);
    m_PromptLabel->SetZIndex(GameConfig::Z_UI);

    // Initially invisible
    Hide();
}

void GameWonPanel::Register(Util::Renderer& renderer) {
    renderer.AddChild(m_Logo);
    renderer.AddChild(m_VictoryLabel);
    renderer.AddChild(m_MarioImage);
    renderer.AddChild(m_PrincessImage);
    renderer.AddChild(m_ScoreLabel);
    renderer.AddChild(m_MessageLabel);
    renderer.AddChild(m_CreditLabel);
    renderer.AddChild(m_PromptLabel);
}

void GameWonPanel::Show() {
    m_Logo->SetVisible(true);
    m_VictoryLabel->SetVisible(true);
    m_MarioImage->SetVisible(true);
    m_PrincessImage->SetVisible(true);
    m_ScoreLabel->SetVisible(true);
    m_MessageLabel->SetVisible(true);
    m_CreditLabel->SetVisible(true);
    m_PromptLabel->SetVisible(true);
}

void GameWonPanel::Hide() {
    m_Logo->SetVisible(false);
    m_VictoryLabel->SetVisible(false);
    m_MarioImage->SetVisible(false);
    m_PrincessImage->SetVisible(false);
    m_ScoreLabel->SetVisible(false);
    m_MessageLabel->SetVisible(false);
    m_CreditLabel->SetVisible(false);
    m_PromptLabel->SetVisible(false);
}

void GameWonPanel::Refresh(const GameStateManager& gs) {
    // Top logo (centered high up, scaled to 0.4f)
    m_Logo->SetPosition(0.0f, 160.0f);

    // Title label
    m_VictoryLabel->SetPosition(0.0f, 30.0f);

    // Side-by-side sprites facing each other (spaced nicely)
    m_MarioImage->SetPosition(-80.0f, -60.0f);
    m_PrincessImage->SetPosition(80.0f, -60.0f);

    // Score readout
    char scoreStr[64];
    snprintf(scoreStr, sizeof(scoreStr), "FINAL SCORE: %06d", gs.GetScore());
    m_ScoreLabel->SetTextContent(scoreStr);
    m_ScoreLabel->SetPosition(0.0f, -150.0f);

    // Message
    m_MessageLabel->SetPosition(0.0f, -200.0f);

    // Creator Credit
    m_CreditLabel->SetPosition(0.0f, -240.0f);

    // Action Prompt
    m_PromptLabel->SetPosition(0.0f, -300.0f);

    // Dynamic retro blinking prompt
    m_FrameCount++;
    m_PromptLabel->SetVisible((m_FrameCount % 60) < 35);
}

}  // namespace Mario
