/**
 * @file AxeEndingPanel.cpp
 * @brief Axe ending panel implementation.
 * @inheritance IUIPanel <- AxeEndingPanel
 */
#include "Mario/UI/AxeEndingPanel.hpp"

#include "Util/Color.hpp"

namespace Mario {

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

}  // namespace Mario
