/**
 * @file ESCMenuPanel.cpp
 * @brief Pause menu panel overlay implementation.
 * @inheritance IUIPanel <- ESCMenuPanel
 */
#include "Mario/UI/ESCMenuPanel.hpp"

#include "Util/Color.hpp"

namespace Mario {

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

}  // namespace Mario
