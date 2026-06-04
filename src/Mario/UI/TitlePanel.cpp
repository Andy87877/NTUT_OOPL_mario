/**
 * @file TitlePanel.cpp
 * @brief Title panel for the start menu.
 * @inheritance IUIPanel <- TitlePanel
 */
#include "Mario/UI/TitlePanel.hpp"

#include "Mario/Core/GameConfig.hpp"
#include "Util/Color.hpp"
#include "config.hpp"

namespace Mario {

TitlePanel::TitlePanel(const std::string& fontPath, int fontSize) {
    auto white = Util::Color::FromRGB(255, 255, 255);

    // Load game logo image
    std::string logoPath = std::string(RESOURCE_DIR) + "/Sprites/logo.png";
    m_Logo = std::make_shared<UIImage>(logoPath);
    m_Logo->m_Transform.scale = {0.4f, 0.4f};
    m_Logo->SetZIndex(GameConfig::Z_UI);

    // Menu options
    m_OnePlayerLabel = std::make_shared<UIText>(fontPath, fontSize, "1 PLAYER GAME", white);
    m_OnePlayerLabel->SetZIndex(GameConfig::Z_UI);

    m_QuitLabel = std::make_shared<UIText>(fontPath, fontSize, "QUIT GAME", white);
    m_QuitLabel->SetZIndex(GameConfig::Z_UI);

    // Menu Selection Cursor (Mushroom or hand icon)
    std::string cursorPath = std::string(RESOURCE_DIR) + "/Sprites/MenuSelect.png";
    m_Cursor = std::make_shared<UIImage>(cursorPath);
    m_Cursor->m_Transform.scale = {GameConfig::DRAW_SCALE, GameConfig::DRAW_SCALE};
    m_Cursor->SetZIndex(GameConfig::Z_UI);

    // Instruction subtitle
    m_SubLabel = std::make_shared<UIText>(
        fontPath, fontSize, "PRESS W/S TO CHOOSE - ENTER TO START", white);
    m_SubLabel->SetZIndex(GameConfig::Z_UI);

    // Traditional Chinese developer credit label
    std::string chineseFontPath = GameConfig::GetChineseFontPath(fontPath);
    m_CreditLabel = std::make_shared<UIText>(
        chineseFontPath, fontSize, "開發者: 113820033 電資二 謝奕宏", white);
    m_CreditLabel->SetZIndex(GameConfig::Z_UI);

    Hide();
}

void TitlePanel::SetMenuContext(int selection) {
    m_Selection = selection;
}

void TitlePanel::Register(Util::Renderer& renderer) {
    renderer.AddChild(m_Logo);
    renderer.AddChild(m_OnePlayerLabel);
    renderer.AddChild(m_QuitLabel);
    renderer.AddChild(m_Cursor);
    renderer.AddChild(m_SubLabel);
    renderer.AddChild(m_CreditLabel);
}

void TitlePanel::Show() {
    m_Logo->SetVisible(true);
    m_OnePlayerLabel->SetVisible(true);
    m_QuitLabel->SetVisible(true);
    m_Cursor->SetVisible(true);
    m_SubLabel->SetVisible(true);
    m_CreditLabel->SetVisible(true);
}

void TitlePanel::Hide() {
    m_Logo->SetVisible(false);
    m_OnePlayerLabel->SetVisible(false);
    m_QuitLabel->SetVisible(false);
    m_Cursor->SetVisible(false);
    m_SubLabel->SetVisible(false);
    m_CreditLabel->SetVisible(false);
}

void TitlePanel::Refresh([[maybe_unused]] const GameStateManager& gs) {
    // Top logo (centered high up, scaled to 0.4f)
    m_Logo->SetPosition(0.0f, 160.0f);

    // Selection options (spaced nicely below the scaled-down logo)
    m_OnePlayerLabel->SetPosition(0.0f, -40.0f);
    m_QuitLabel->SetPosition(0.0f, -90.0f);

    // Subtitle instruction
    m_SubLabel->SetPosition(0.0f, -190.0f);

    // Dynamic retro blinking prompt
    m_FrameCount++;
    m_SubLabel->SetVisible((m_FrameCount % 60) < 35);

    // Position credit label centered nicely at the bottom
    m_CreditLabel->SetPosition(0.0f, -240.0f);

    // Position cursor precisely to the left of the selected label
    if (m_Selection == 0) {
        m_Cursor->SetPosition(-150.0f, -40.0f);
    } else {
        m_Cursor->SetPosition(-150.0f, -90.0f);
    }
}

}  // namespace Mario
