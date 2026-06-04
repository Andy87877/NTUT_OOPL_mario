/**
 * @file UIManager.cpp
 * @brief Implementation of UIManager.
 * @inheritance None (Manager class)
 */
#include "Mario/UI/UIManager.hpp"

#include <cstdio>
#include <fstream>

#include "Mario/Core/GameConfig.hpp"
#include "Util/Color.hpp"
#include "Util/Logger.hpp"
#include "config.hpp"

namespace Mario {

UIManager::UIManager(GameStateManager* gameState)
    : m_GameState(gameState),
      m_HUDPanel(m_FontPath, m_FontSize),
      m_TitlePanel(m_FontPath, m_FontSize),
      m_LoadingPanel(m_FontPath, m_FontSize),
      m_GameOverPanel(m_FontPath, m_FontSize),
      m_GameWonPanel(m_FontPath, m_FontSize),
      m_ESCMenuPanel(m_FontPath, m_FontSize),
      m_AxeEndingPanel(m_FontPath, m_FontSize) {
    auto white = Util::Color::FromRGB(255, 255, 255);

    // Register all panels into the shared renderer.
    m_HUDPanel.Register(m_UIRenderer);
    m_TitlePanel.Register(m_UIRenderer);
    m_LoadingPanel.Register(m_UIRenderer);
    m_GameOverPanel.Register(m_UIRenderer);
    m_GameWonPanel.Register(m_UIRenderer);
    m_ESCMenuPanel.Register(m_UIRenderer);
    m_AxeEndingPanel.Register(m_UIRenderer);

    // Build panel dispatch map (HUD is handled separately as it overlays
    // gameplay states; only scene-specific panels are in the map).
    m_PanelMap = {
        {State::TITLE, &m_TitlePanel},
        {State::LOADING, &m_LoadingPanel},
        {State::GAME_OVER, &m_GameOverPanel},
        {State::GAME_WON, &m_GameWonPanel},
        {State::ESC_MENU, &m_ESCMenuPanel},
        {State::AXE_SEQUENCE, &m_AxeEndingPanel},
    };

    // FPS counter (bottom-right corner).
    m_FPSText =
        std::make_shared<UIText>(m_FontPath, m_FontSize, "FPS: --", white);
    m_FPSText->SetPosition(520.0f, -340.0f);
    m_UIRenderer.AddChild(m_FPSText);

    // Copyright text (bottom-left corner); prefer a Chinese-capable font.
    std::string chineseFontPath = GameConfig::GetChineseFontPath(m_FontPath);
    m_CopyrightText = std::make_shared<UIText>(
        chineseFontPath, m_FontSize, "113820033 電資二 謝奕宏", white);
    m_CopyrightText->SetPosition(-620.0f, -340.0f);
    m_UIRenderer.AddChild(m_CopyrightText);

    // Construct gold-colored Cheat Mode text centered at the bottom of the screen
    m_CheatModeText = std::make_shared<UIText>(
        m_FontPath, m_FontSize, "CHEAT MODE ENABLED", Util::Color::FromRGB(255, 215, 0));
    m_CheatModeText->SetPosition(0.0f, -340.0f);
    m_CheatModeText->SetVisible(false);
    m_UIRenderer.AddChild(m_CheatModeText);
}

void UIManager::HideAllScenePanels() {
    for (auto& [state, panel] : m_PanelMap) {
        panel->Hide();
    }
}

void UIManager::Update(State currentState, int escMenuSelection,
                       const std::string& powerStateName) {
    if (!m_GameState) return;

    // --- FPS counter ---
    m_FrameCount++;
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                       now - m_LastFPSTime)
                       .count();
    if (elapsed >= 1000) {
        m_FPS = static_cast<int>(m_FrameCount * 1000.0f / elapsed);
        m_FrameCount = 0;
        m_LastFPSTime = now;
        char buf[32];
        snprintf(buf, sizeof(buf), "FPS: %d", m_FPS);
        m_FPSText->SetTextContent(buf);
    }

    // --- HUD visibility (shown during all active gameplay states) ---
    bool showHUD =
        (currentState == State::PLAYING || currentState == State::ESC_MENU);
    if (showHUD) {
        m_HUDPanel.Show();
        m_HUDPanel.Refresh(*m_GameState);
    } else {
        m_HUDPanel.Hide();
    }

    // --- Cheat Mode bottom UI visibility ---
    if (m_GameState && m_GameState->IsCheatModeActive() && showHUD) {
        m_CheatModeText->SetVisible(true);
    } else {
        m_CheatModeText->SetVisible(false);
    }

    // --- Scene panel dispatch ---
    if (m_FirstUpdate || m_LastState != currentState) {
        HideAllScenePanels();
        auto itNew = m_PanelMap.find(currentState);
        if (itNew != m_PanelMap.end()) {
            itNew->second->Show();
        }
        m_LastState = currentState;
        m_FirstUpdate = false;
    }

    auto it = m_PanelMap.find(currentState);
    if (it != m_PanelMap.end()) {
        IUIPanel* panel = it->second;

        // Supply extra context to panels that need it before Refresh().
        if (currentState == State::TITLE) {
            m_TitlePanel.SetMenuContext(escMenuSelection);
        }
        if (currentState == State::ESC_MENU) {
            m_ESCMenuPanel.SetMenuContext(escMenuSelection, powerStateName);
        }
        if (currentState == State::AXE_SEQUENCE) {
            m_AxeEndingPanel.SetShowCredits(m_EndingTextPhase ==
                                             EndingTextPhase::CREDITS);
        }

        panel->Refresh(*m_GameState);
    }

    // --- Floating text (score pop-ups, 1UP, etc.) ---
    if (!m_FloatingTexts.empty()) {
        auto it2 = m_FloatingTexts.begin();
        while (it2 != m_FloatingTexts.end()) {
            (*it2)->Update();
            if ((*it2)->IsExpired()) {
                m_UIRenderer.RemoveChild((*it2)->GetUIText());
                it2 = m_FloatingTexts.erase(it2);
            } else {
                ++it2;
            }
        }
    }

    m_UIRenderer.Update();
}

void UIManager::AddFloatingText(float screenX, float screenY,
                                const std::string& text, int frames) {
    auto ft = std::make_shared<FloatingText>(screenX, screenY, text, frames);
    m_FloatingTexts.push_back(ft);
    m_UIRenderer.AddChild(ft->GetUIText());
    LOG_DEBUG("Added floating text: '{}' at screen ({}, {})", text, screenX,
              screenY);
}

}  // namespace Mario
