/**
 * @file SimpleTextPanel.cpp
 * @brief Simple text panel implementation.
 * @inheritance IUIPanel <- SimpleTextPanel
 */
#include "Mario/UI/SimpleTextPanel.hpp"

#include <cstdio>

#include "Util/Color.hpp"

namespace Mario {

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

}  // namespace Mario
