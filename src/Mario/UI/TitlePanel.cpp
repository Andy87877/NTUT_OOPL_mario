/**
 * @file TitlePanel.cpp
 * @brief Title panel implementation.
 * @inheritance IUIPanel <- TitlePanel
 */
#include "Mario/UI/TitlePanel.hpp"

#include "Util/Color.hpp"

namespace Mario {

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

}  // namespace Mario
