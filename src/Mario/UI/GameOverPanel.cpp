/**
 * @file GameOverPanel.cpp
 * @brief Dedicated retro GameOver screen panel implementation.
 * @inheritance IUIPanel <- GameOverPanel
 */
#include "Mario/UI/GameOverPanel.hpp"

#include <cstdio>

#include "Mario/Core/GameConfig.hpp"
#include "Util/Color.hpp"
#include "config.hpp"

namespace Mario {

GameOverPanel::GameOverPanel(const std::string& fontPath, int fontSize) {
    auto white = Util::Color::FromRGB(255, 255, 255);
    auto red = Util::Color::FromRGB(255, 60, 60);

    // Load game logo
    std::string logoPath = std::string(RESOURCE_DIR) + "/Sprites/logo.png";
    m_Logo = std::make_shared<UIImage>(logoPath);
    m_Logo->m_Transform.scale = {0.35f, 0.35f};
    m_Logo->SetZIndex(GameConfig::Z_UI);

    // GameOver Label
    m_GameOverLabel = std::make_shared<UIText>(fontPath, fontSize * 2, "GAME OVER", red);
    m_GameOverLabel->SetZIndex(GameConfig::Z_UI);

    // Score readout
    m_ScoreLabel = std::make_shared<UIText>(fontPath, fontSize, "", white);
    m_ScoreLabel->SetZIndex(GameConfig::Z_UI);

    // Prompt to return to title
    m_PromptLabel = std::make_shared<UIText>(
        fontPath, fontSize, "PRESS ANY KEY TO RETURN TO TITLE", white);
    m_PromptLabel->SetZIndex(GameConfig::Z_UI);

    Hide();
}

void GameOverPanel::Register(Util::Renderer& renderer) {
    renderer.AddChild(m_Logo);
    renderer.AddChild(m_GameOverLabel);
    renderer.AddChild(m_ScoreLabel);
    renderer.AddChild(m_PromptLabel);
}

void GameOverPanel::Show() {
    m_Logo->SetVisible(true);
    m_GameOverLabel->SetVisible(true);
    m_ScoreLabel->SetVisible(true);
    m_PromptLabel->SetVisible(true);
}

void GameOverPanel::Hide() {
    m_Logo->SetVisible(false);
    m_GameOverLabel->SetVisible(false);
    m_ScoreLabel->SetVisible(false);
    m_PromptLabel->SetVisible(false);
}

void GameOverPanel::Refresh(const GameStateManager& gs) {
    // Logo position
    m_Logo->SetPosition(0.0f, 180.0f);

    // "GAME OVER" label position
    m_GameOverLabel->SetPosition(0.0f, 10.0f);

    // Score display position
    char scoreStr[64];
    snprintf(scoreStr, sizeof(scoreStr), "FINAL SCORE: %06d", gs.GetScore());
    m_ScoreLabel->SetTextContent(scoreStr);
    m_ScoreLabel->SetPosition(0.0f, -60.0f);

    // Prompt position
    m_PromptLabel->SetPosition(0.0f, -150.0f);

    // Dynamic retro blinking prompt
    m_FrameCount++;
    m_PromptLabel->SetVisible((m_FrameCount % 60) < 35);
}

}  // namespace Mario
