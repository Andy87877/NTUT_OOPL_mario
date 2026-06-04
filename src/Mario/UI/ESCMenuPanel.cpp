/**
 * @file ESCMenuPanel.cpp
 * @brief Pause menu panel overlay implementation.
 * @inheritance IUIPanel <- ESCMenuPanel
 */
#include "Mario/UI/ESCMenuPanel.hpp"

#include "Mario/Core/GameConfig.hpp"
#include "Util/Color.hpp"
#include "config.hpp"

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

    auto gold = Util::Color::FromRGB(255, 215, 0);
    auto grey = Util::Color::FromRGB(160, 160, 160);
    m_DescLabel = std::make_shared<UIText>(fontPath, fontSize * 0.7f, "", gold);
    m_DescLabel->SetVisible(false);
    m_HintLabel = std::make_shared<UIText>(fontPath, fontSize * 0.65f, "[UP/DOWN] MOVE   [ENTER] SELECT   [ESC] RESUME", grey);
    m_HintLabel->SetVisible(false);

    std::string cursorPath = std::string(RESOURCE_DIR) + "/Sprites/MenuSelect.png";
    m_Cursor = std::make_shared<UIImage>(cursorPath);
    m_Cursor->m_Transform.scale = {GameConfig::DRAW_SCALE, GameConfig::DRAW_SCALE};
    m_Cursor->SetZIndex(GameConfig::Z_UI);
    m_Cursor->SetVisible(false);

    std::string overlayPath = std::string(RESOURCE_DIR) + "/Sprites/DarkOverlay.png";
    m_Overlay = std::make_shared<UIImage>(overlayPath);
    m_Overlay->m_Transform.scale = {1280.0f, 720.0f};
    m_Overlay->SetZIndex(80.0f); // Render above game blocks/entities (Z <= 10) but below menu text/cursor (Z >= 90)
    m_Overlay->SetVisible(false);
}

void ESCMenuPanel::SetMenuContext(int selection,
                                  const std::string& powerStateName) {
    m_Selection = selection;
    m_PowerStateName = powerStateName;
}

void ESCMenuPanel::Register(Util::Renderer& renderer) {
    renderer.AddChild(m_Overlay);
    renderer.AddChild(m_PausedLabel);
    renderer.AddChild(m_DescLabel);
    renderer.AddChild(m_HintLabel);
    renderer.AddChild(m_Cursor);
    for (auto& t : m_MenuTexts) {
        renderer.AddChild(t);
    }
}

void ESCMenuPanel::Show() {
    m_Overlay->SetVisible(true);
    m_PausedLabel->SetVisible(true);
    m_DescLabel->SetVisible(true);
    m_HintLabel->SetVisible(true);
    m_Cursor->SetVisible(true);
    for (auto& t : m_MenuTexts) {
        t->SetVisible(true);
    }
}

void ESCMenuPanel::Hide() {
    m_Overlay->SetVisible(false);
    m_PausedLabel->SetVisible(false);
    m_DescLabel->SetVisible(false);
    m_HintLabel->SetVisible(false);
    m_Cursor->SetVisible(false);
    for (auto& t : m_MenuTexts) {
        t->SetVisible(false);
    }
}

void ESCMenuPanel::Refresh(const GameStateManager& gs) {
    m_Overlay->SetPosition(0.0f, 0.0f);
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

    // Set dynamic description based on current selection (Context-sensitive UI/UX)
    std::string desc = "";
    switch (m_Selection) {
        case 0: desc = "> RESUME GAME AND RETURN TO PLAYING"; break;
        case 1: desc = "> WARP IMMEDIATELY TO WORLD 1-1"; break;
        case 2: desc = "> WARP IMMEDIATELY TO WORLD 1-2"; break;
        case 3: desc = "> WARP IMMEDIATELY TO WORLD 8-4"; break;
        case 4: desc = "> CYCLE MARIO'S POWER LEVEL STATE"; break;
        case 5: desc = "> TOGGLE INFINITE LIVES & STAR CHEAT"; break;
        default: desc = ""; break;
    }
    m_DescLabel->SetTextContent(desc);
    m_DescLabel->SetPosition(0.0f, -220.0f);

    m_HintLabel->SetPosition(0.0f, -280.0f);

    m_Cursor->SetPosition(-160.0f, startY - static_cast<float>(m_Selection) * 55.0f);
}

}  // namespace Mario
