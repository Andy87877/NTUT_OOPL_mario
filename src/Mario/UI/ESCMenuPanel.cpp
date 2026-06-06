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

    std::vector<std::string> options = {"RESUME", "CONTROLS", "1-1", "1-2", "8-4",
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

    // Controls guide UI elements
    m_ControlsTitle = std::make_shared<UIText>(fontPath, fontSize * 1.5f, "CONTROLS GUIDE", gold);
    m_ControlsTitle->SetZIndex(GameConfig::Z_UI + 5.0f);

    std::string chineseFontPath = GameConfig::GetChineseFontPath(fontPath);

    // Instantiate column headers with larger font size and simplified text
    m_HeaderEng = std::make_shared<UIText>(fontPath, fontSize * 0.9f, "ACTION", gold);
    m_HeaderEng->SetZIndex(GameConfig::Z_UI + 5.0f);
    m_HeaderChi = std::make_shared<UIText>(chineseFontPath, fontSize * 0.9f, "操作", gold);
    m_HeaderChi->SetZIndex(GameConfig::Z_UI + 5.0f);
    m_HeaderKey = std::make_shared<UIText>(fontPath, fontSize * 0.9f, "KEYS", gold);
    m_HeaderKey->SetZIndex(GameConfig::Z_UI + 5.0f);

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
        auto engWidget = std::make_shared<UIText>(fontPath, fontSize * 0.85f, ctrl.eng, white);
        engWidget->SetZIndex(GameConfig::Z_UI + 5.0f);
        m_ControlsEng.push_back(engWidget);

        auto chiWidget = std::make_shared<UIText>(chineseFontPath, fontSize * 0.85f, ctrl.chi, white);
        chiWidget->SetZIndex(GameConfig::Z_UI + 5.0f);
        m_ControlsChi.push_back(chiWidget);

        auto keyWidget = std::make_shared<UIText>(fontPath, fontSize * 0.85f, ctrl.key, white);
        keyWidget->SetZIndex(GameConfig::Z_UI + 5.0f);
        m_ControlsKey.push_back(keyWidget);
    }

    m_ControlsBackHint = std::make_shared<UIText>(
        fontPath, fontSize * 0.75f, "PRESS ENTER OR ESC TO RETURN", grey);
    m_ControlsBackHint->SetZIndex(GameConfig::Z_UI + 5.0f);
}

void ESCMenuPanel::SetMenuContext(int selection,
                                  const std::vector<std::string>& itemTexts,
                                  const std::string& description) {
    m_Selection = selection;
    m_Description = description;

    for (size_t i = 0; i < itemTexts.size(); ++i) {
        if (i < m_MenuTexts.size()) {
            m_MenuTexts[i]->SetTextContent(itemTexts[i]);
        }
    }
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

    renderer.AddChild(m_ControlsTitle);
    renderer.AddChild(m_HeaderEng);
    renderer.AddChild(m_HeaderChi);
    renderer.AddChild(m_HeaderKey);
    for (auto& eng : m_ControlsEng) renderer.AddChild(eng);
    for (auto& chi : m_ControlsChi) renderer.AddChild(chi);
    for (auto& key : m_ControlsKey) renderer.AddChild(key);
    renderer.AddChild(m_ControlsBackHint);
}

void ESCMenuPanel::Show() {
    m_Overlay->SetVisible(true);
    if (m_ShowControls) {
        m_PausedLabel->SetVisible(false);
        m_DescLabel->SetVisible(false);
        m_HintLabel->SetVisible(false);
        m_Cursor->SetVisible(false);
        for (auto& t : m_MenuTexts) {
            t->SetVisible(false);
        }

        m_ControlsTitle->SetVisible(true);
        m_HeaderEng->SetVisible(true);
        m_HeaderChi->SetVisible(true);
        m_HeaderKey->SetVisible(true);
        for (auto& eng : m_ControlsEng) eng->SetVisible(true);
        for (auto& chi : m_ControlsChi) chi->SetVisible(true);
        for (auto& key : m_ControlsKey) key->SetVisible(true);
        m_ControlsBackHint->SetVisible(true);
    } else {
        m_PausedLabel->SetVisible(true);
        m_DescLabel->SetVisible(true);
        m_HintLabel->SetVisible(true);
        m_Cursor->SetVisible(true);
        for (auto& t : m_MenuTexts) {
            t->SetVisible(true);
        }

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

void ESCMenuPanel::Hide() {
    m_Overlay->SetVisible(false);
    m_PausedLabel->SetVisible(false);
    m_DescLabel->SetVisible(false);
    m_HintLabel->SetVisible(false);
    m_Cursor->SetVisible(false);
    for (auto& t : m_MenuTexts) {
        t->SetVisible(false);
    }

    m_ControlsTitle->SetVisible(false);
    m_HeaderEng->SetVisible(false);
    m_HeaderChi->SetVisible(false);
    m_HeaderKey->SetVisible(false);
    for (auto& eng : m_ControlsEng) eng->SetVisible(false);
    for (auto& chi : m_ControlsChi) chi->SetVisible(false);
    for (auto& key : m_ControlsKey) key->SetVisible(false);
    m_ControlsBackHint->SetVisible(false);
}

void ESCMenuPanel::Refresh(const GameStateManager&) {
    m_Overlay->SetPosition(0.0f, 0.0f);
    
    // Synchronize visibility of all components based on current substate
    Show();

    if (!m_ShowControls) {
        m_PausedLabel->SetPosition(0.0f, 280.0f);

        float startY = 130.0f;
        float spacing = 48.0f;
        for (size_t i = 0; i < m_MenuTexts.size(); ++i) {
            m_MenuTexts[i]->SetPosition(0.0f,
                                        startY - static_cast<float>(i) * spacing);
            auto color = (static_cast<int>(i) == m_Selection)
                              ? Util::Color::FromRGB(255, 0, 0)       // highlighted
                              : Util::Color::FromRGB(255, 255, 255);  // normal
            m_MenuTexts[i]->SetTextColor(color);
        }

        m_DescLabel->SetTextContent(m_Description);
        m_DescLabel->SetPosition(0.0f, -220.0f);

        m_HintLabel->SetPosition(0.0f, -280.0f);

        m_Cursor->SetPosition(-180.0f, startY - static_cast<float>(m_Selection) * spacing);
    } else {
        m_ControlsTitle->SetPosition(0.0f, 200.0f);

        // Position column headers
        m_HeaderEng->SetPosition(-300.0f, 130.0f);
        m_HeaderChi->SetPosition(50.0f, 130.0f);
        m_HeaderKey->SetPosition(360.0f, 130.0f);

        float startY = 70.0f;
        float spacing = 40.0f;
        for (size_t i = 0; i < m_ControlsEng.size(); ++i) {
            float y = startY - static_cast<float>(i) * spacing;
            m_ControlsEng[i]->SetPosition(-300.0f, y);
            m_ControlsChi[i]->SetPosition(50.0f, y);
            m_ControlsKey[i]->SetPosition(360.0f, y);
        }

        m_ControlsBackHint->SetPosition(0.0f, -230.0f);

        m_FrameCount++;
        m_ControlsBackHint->SetVisible((m_FrameCount % 60) < 35);
    }
}

}  // namespace Mario
