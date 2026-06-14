/**
 * @file MenuSceneHandlers.cpp
 * @brief Implementations for all simple menu/transition scene handlers.
 *        Also includes ISceneHandler's default OnRender() implementation.
 *        Merged from: ISceneHandler.cpp, TitleSceneHandler.cpp,
 *        DeathSceneHandler.cpp, GameOverSceneHandler.cpp,
 *        GameWonSceneHandler.cpp.
 * @inheritance ISceneHandler <- TitleSceneHandler
 *              ISceneHandler <- DeathSceneHandler
 *              ISceneHandler <- GameOverSceneHandler
 *              ISceneHandler <- GameWonSceneHandler
 */
#include "Mario/Scenes/MenuSceneHandlers.hpp"

#include "App.hpp"
#include "Mario/Services/AudioManager.hpp"
#include "Mario/UI/TitleMenuItems.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include "Util/Logger.hpp"

namespace Mario {

// ============================================================================
// ISceneHandler — default OnRender() (was ISceneHandler.cpp)
// ============================================================================

void ISceneHandler::OnRender(App& app) {
    // Default: flush the renderer with no UI override.
    // Concrete handlers call app.ApplyBackground() + UIManager as needed.
    app.GetRenderer().Update();
}

// ============================================================================
// TitleSceneHandler
// ============================================================================

TitleSceneHandler::TitleSceneHandler() {
    m_MenuItems.push_back(std::make_unique<StartGameMenuItem>());
    m_MenuItems.push_back(std::make_unique<ControlsMenuItem>());
    m_MenuItems.push_back(std::make_unique<QuitGameMenuItem>());
}

TitleSceneHandler::~TitleSceneHandler() = default;

void TitleSceneHandler::OnEnter(App& app) {
    Mario::AudioManager::GetInstance().PlayBGM(Mario::BGMName::NameEntryVsSuperMarioBros);
}

void TitleSceneHandler::Update(App& app) {
    if (m_SubState == SubState::MENU) {
        int size = static_cast<int>(m_MenuItems.size());
        bool upDown = Util::Input::IsKeyDown(Util::Keycode::UP);
        bool wDown = Util::Input::IsKeyDown(Util::Keycode::W);
        if (upDown || wDown) {
            m_Selection = (m_Selection - 1 + size) % size;
        }
        bool downDown = Util::Input::IsKeyDown(Util::Keycode::DOWN);
        bool sDown = Util::Input::IsKeyDown(Util::Keycode::S);
        if (downDown || sDown) {
            m_Selection = (m_Selection + 1) % size;
        }

        if (Util::Input::IsKeyDown(Util::Keycode::RETURN)) {
            if (m_Selection >= 0 && m_Selection < size) {
                m_MenuItems[m_Selection]->Execute(app, *this);
            }
        }
        if (Util::Input::IsKeyDown(Util::Keycode::ESCAPE)) {
            app.TransitionTo(App::State::END);
        }
    } else if (m_SubState == SubState::CONTROLS) {
        if (Util::Input::IsKeyDown(Util::Keycode::RETURN) ||
            Util::Input::IsKeyDown(Util::Keycode::ESCAPE)) {
            m_SubState = SubState::MENU;
            LOG_INFO("Returning to title menu");
        }
    }
}

void TitleSceneHandler::OnRender(App& app) {
    app.ApplyBackground(false);  // Sky-blue title screen
    app.GetRenderer().Update();

    std::vector<std::string> displayTexts;
    for (const auto& item : m_MenuItems) {
        displayTexts.push_back(item->GetDisplayText(app));
    }

    auto& panel = app.GetUIManager().GetTitlePanel();
    panel.SetShowControls(m_SubState == SubState::CONTROLS);
    panel.SetMenuContext(m_Selection, displayTexts);
    app.GetUIManager().Update(Mario::UIManager::State::TITLE);
}

// ============================================================================
// DeathSceneHandler
// ============================================================================

void DeathSceneHandler::OnEnter(App& app) {
    app.GetGameState().LoseLife();
    auto& player = app.GetPlayer();
    if (player) {
        player->GetState().StartDeathAnimation();
    }
    app.GetDeathTimer() = app.GetTimer() + 150;
    Mario::AudioManager::GetInstance().PlayBGM(Mario::BGMName::LostALifeTheme);
    LOG_INFO("Player died - entering DEATH state (Lives remaining: {})",
             app.GetGameState().GetLives());
}

void DeathSceneHandler::Update(App& app) {
    auto& player = app.GetPlayer();
    if (player) {
        player->GetState().UpdateDeathAnimation();
        player->UpdateView(app.GetCamera().GetOffset());
    }

    if (app.GetTimer() > app.GetDeathTimer()) {
        if (!app.GetGameState().IsGameOver()) {
            app.TransitionTo(App::State::LOADING);
        } else {
            app.TransitionTo(App::State::GAME_OVER);
            Mario::AudioManager::GetInstance().PlayBGM(
                Mario::BGMName::GameOverTheme);
            LOG_INFO("No lives remaining - GAME_OVER");
        }
    }
}

void DeathSceneHandler::OnRender(App& app) {
    app.ApplyBackground();
    app.GetRenderer().Update();
    app.GetUIManager().Update(Mario::UIManager::State::PLAYING);
}

// ============================================================================
// GameOverSceneHandler
// ============================================================================

void GameOverSceneHandler::OnEnter(App& app) {
    Mario::AudioManager::GetInstance().PlayBGM(Mario::BGMName::GameOverTheme);
}

void GameOverSceneHandler::Update(App& app) {
    if (Util::Input::IsAnyKeyDown()) {
        app.TransitionTo(App::State::TITLE);
        LOG_INFO("Game Over - returning to TITLE");
    }
}

void GameOverSceneHandler::OnRender(App& app) {
    app.ApplyBackground(true);  // Black screen
    app.GetRenderer().Update();
    app.GetUIManager().Update(Mario::UIManager::State::GAME_OVER);
}

// ============================================================================
// GameWonSceneHandler
// ============================================================================

void GameWonSceneHandler::OnEnter(App& app) {
    Mario::AudioManager::GetInstance().PlayBGM(Mario::BGMName::SavedThePrincessNes);
}

void GameWonSceneHandler::Update(App& app) {
    bool enterDown = Util::Input::IsKeyDown(Util::Keycode::RETURN);
    bool escDown = Util::Input::IsKeyDown(Util::Keycode::ESCAPE);
    if (enterDown || escDown) {
        app.TransitionTo(App::State::TITLE);
        LOG_INFO("Game Won! Returning to TITLE screen.");
    }
}

void GameWonSceneHandler::OnRender(App& app) {
    app.ApplyBackground(true);  // Solid black background for victory screen
    app.GetUIManager().Update(Mario::UIManager::State::GAME_WON);
}

}  // namespace Mario
