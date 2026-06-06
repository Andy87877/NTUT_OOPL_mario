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

    // Menu options initialized dynamically
    std::vector<std::string> options = {"1 PLAYER GAME", "CONTROLS", "QUIT GAME"};
    for (const auto& opt : options) {
        auto text = std::make_shared<UIText>(fontPath, fontSize, opt, white);
        text->SetZIndex(GameConfig::Z_UI);
        text->SetVisible(false);
        m_MenuTexts.push_back(text);
    }

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

    // Controls guide UI elements
    auto gold = Util::Color::FromRGB(255, 215, 0);
    auto grey = Util::Color::FromRGB(160, 160, 160);

    m_ControlsTitle = std::make_shared<UIText>(fontPath, fontSize * 1.6f, "CONTROLS GUIDE", gold);
    m_ControlsTitle->SetZIndex(GameConfig::Z_UI);

    // Instantiate column headers with larger font size and simplified text
    m_HeaderEng = std::make_shared<UIText>(fontPath, fontSize * 0.95f, "ACTION", gold);
    m_HeaderEng->SetZIndex(GameConfig::Z_UI);
    m_HeaderChi = std::make_shared<UIText>(chineseFontPath, fontSize * 0.95f, "操作", gold);
    m_HeaderChi->SetZIndex(GameConfig::Z_UI);
    m_HeaderKey = std::make_shared<UIText>(fontPath, fontSize * 0.95f, "KEYS", gold);
    m_HeaderKey->SetZIndex(GameConfig::Z_UI);

    struct ControlInfo {
        std::string eng;
        std::string chi;
        std::string key;
    };

    // Separated Sprint & Fireball, removed Z from JUMP
    std::vector<ControlInfo> controlsList = {
        {"MOVE LEFT/RIGHT", "左右移動", "<- ->  OR  A / D"},
        {"JUMP", "跳躍", "W / SPACE / ↑"},
        {"CROUCH", "蹲下", "S / ↓"},
        {"SPRINT", "加速", "LSHIFT"},
        {"FIREBALL", "發射火球", "E"},
        {"PAUSE MENU", "暫停選單", "ESC"},
        {"CONFIRM", "確認", "ENTER"}
    };

    for (const auto& ctrl : controlsList) {
        auto engWidget = std::make_shared<UIText>(fontPath, fontSize * 0.9f, ctrl.eng, white);
        engWidget->SetZIndex(GameConfig::Z_UI);
        m_ControlsEng.push_back(engWidget);

        auto chiWidget = std::make_shared<UIText>(chineseFontPath, fontSize * 0.9f, ctrl.chi, white);
        chiWidget->SetZIndex(GameConfig::Z_UI);
        m_ControlsChi.push_back(chiWidget);

        auto keyWidget = std::make_shared<UIText>(fontPath, fontSize * 0.9f, ctrl.key, white);
        keyWidget->SetZIndex(GameConfig::Z_UI);
        m_ControlsKey.push_back(keyWidget);
    }

    m_ControlsBackHint = std::make_shared<UIText>(
        fontPath, fontSize * 0.75f, "PRESS ENTER OR ESC TO RETURN", grey);
    m_ControlsBackHint->SetZIndex(GameConfig::Z_UI);

    Hide();
}

void TitlePanel::SetMenuContext(int selection, const std::vector<std::string>& itemTexts) {
    m_Selection = selection;
    for (size_t i = 0; i < itemTexts.size(); ++i) {
        if (i < m_MenuTexts.size()) {
            m_MenuTexts[i]->SetTextContent(itemTexts[i]);
        }
    }
}

void TitlePanel::Register(Util::Renderer& renderer) {
    renderer.AddChild(m_Logo);
    for (auto& t : m_MenuTexts) {
        renderer.AddChild(t);
    }
    renderer.AddChild(m_Cursor);
    renderer.AddChild(m_SubLabel);
    renderer.AddChild(m_CreditLabel);

    renderer.AddChild(m_ControlsTitle);
    renderer.AddChild(m_HeaderEng);
    renderer.AddChild(m_HeaderChi);
    renderer.AddChild(m_HeaderKey);
    for (auto& eng : m_ControlsEng) renderer.AddChild(eng);
    for (auto& chi : m_ControlsChi) renderer.AddChild(chi);
    for (auto& key : m_ControlsKey) renderer.AddChild(key);
    renderer.AddChild(m_ControlsBackHint);
}

void TitlePanel::Show() {
    if (m_ShowControls) {
        m_Logo->SetVisible(false);
        for (auto& t : m_MenuTexts) t->SetVisible(false);
        m_Cursor->SetVisible(false);
        m_SubLabel->SetVisible(false);
        m_CreditLabel->SetVisible(false);

        m_ControlsTitle->SetVisible(true);
        m_HeaderEng->SetVisible(true);
        m_HeaderChi->SetVisible(true);
        m_HeaderKey->SetVisible(true);
        for (auto& eng : m_ControlsEng) eng->SetVisible(true);
        for (auto& chi : m_ControlsChi) chi->SetVisible(true);
        for (auto& key : m_ControlsKey) key->SetVisible(true);
        m_ControlsBackHint->SetVisible(true);
    } else {
        m_Logo->SetVisible(true);
        for (auto& t : m_MenuTexts) t->SetVisible(true);
        m_Cursor->SetVisible(true);
        m_SubLabel->SetVisible(true);
        m_CreditLabel->SetVisible(true);

        m_ControlsTitle->SetVisible(false);
        m_HeaderEng->SetVisible(false);
        m_HeaderChi->SetVisible(false);
        m_HeaderKey->SetVisible(false);
        for (auto& eng : m_ControlsEng) eng->SetVisible(false);
        for (auto& chi : m_ControlsChi) chi->SetVisible(false);
        for (auto& key : m_ControlsKey) key->SetVisible(false);
        m_ControlsBackHint->SetVisible(false);
    }
}

void TitlePanel::Hide() {
    m_Logo->SetVisible(false);
    for (auto& t : m_MenuTexts) t->SetVisible(false);
    m_Cursor->SetVisible(false);
    m_SubLabel->SetVisible(false);
    m_CreditLabel->SetVisible(false);

    m_ControlsTitle->SetVisible(false);
    m_HeaderEng->SetVisible(false);
    m_HeaderChi->SetVisible(false);
    m_HeaderKey->SetVisible(false);
    for (auto& eng : m_ControlsEng) eng->SetVisible(false);
    for (auto& chi : m_ControlsChi) chi->SetVisible(false);
    for (auto& key : m_ControlsKey) key->SetVisible(false);
    m_ControlsBackHint->SetVisible(false);
}

void TitlePanel::Refresh([[maybe_unused]] const GameStateManager& gs) {
    // Synchronize visibility of all components based on current substate
    Show();

    if (!m_ShowControls) {
        // Top logo (centered high up, scaled to 0.4f)
        m_Logo->SetPosition(0.0f, 160.0f);

        // Selection options (spaced nicely below the scaled-down logo)
        float startY = -20.0f;
        for (size_t i = 0; i < m_MenuTexts.size(); ++i) {
            m_MenuTexts[i]->SetPosition(0.0f, startY - static_cast<float>(i) * 50.0f);
            auto color = (static_cast<int>(i) == m_Selection)
                              ? Util::Color::FromRGB(255, 0, 0)       // highlighted red
                              : Util::Color::FromRGB(255, 255, 255);  // normal white
            m_MenuTexts[i]->SetTextColor(color);
        }

        // Subtitle instruction
        m_SubLabel->SetPosition(0.0f, -190.0f);

        // Dynamic retro blinking prompt
        m_FrameCount++;
        m_SubLabel->SetVisible((m_FrameCount % 60) < 35);

        // Position credit label centered nicely at the bottom
        m_CreditLabel->SetPosition(0.0f, -240.0f);

        // Position cursor precisely to the left of the selected label
        if (m_Selection >= 0 && m_Selection < static_cast<int>(m_MenuTexts.size())) {
            m_Cursor->SetPosition(-170.0f, startY - static_cast<float>(m_Selection) * 50.0f);
        }
    } else {
        // Draw the controls guide
        m_ControlsTitle->SetPosition(0.0f, 220.0f);

        // Position column headers
        m_HeaderEng->SetPosition(-300.0f, 140.0f);
        m_HeaderChi->SetPosition(50.0f, 140.0f);
        m_HeaderKey->SetPosition(360.0f, 140.0f);

        float startY = 90.0f;
        float spacing = 42.0f;
        for (size_t i = 0; i < m_ControlsEng.size(); ++i) {
            float y = startY - static_cast<float>(i) * spacing;
            m_ControlsEng[i]->SetPosition(-300.0f, y);
            m_ControlsChi[i]->SetPosition(50.0f, y);
            m_ControlsKey[i]->SetPosition(360.0f, y);
        }

        m_ControlsBackHint->SetPosition(0.0f, -230.0f);

        // Dynamic retro blinking prompt for the back-to-menu hint
        m_FrameCount++;
        m_ControlsBackHint->SetVisible((m_FrameCount % 60) < 35);
    }
}

}  // namespace Mario
