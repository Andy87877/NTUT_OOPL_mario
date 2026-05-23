/**
 * @file ESCMenuSceneHandler.cpp
 * @brief Pause/ESC menu scene handler implementation.
 *        Handles 5 menu items: RESUME, 1-1, 1-2, 8-4, and the POWER cheat
 *        that cycles Mario's power state (SMALL / BIG / FIRE / STAR).
 * @inheritance ISceneHandler <- ESCMenuSceneHandler
 */
#include "Mario/ESCMenuSceneHandler.hpp"

#include "App.hpp"
#include "Mario/AudioManager.hpp"
#include "Mario/PlayerState.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include "Util/Logger.hpp"

namespace Mario {

// ============================================================================
// GetPowerStateName — maps the cyclic index to a short display name.
// ============================================================================
const char* ESCMenuSceneHandler::GetPowerStateName(int idx) {
    switch (idx) {
        case 0:
            return "SMALL";
        case 1:
            return "BIG";
        case 2:
            return "FIRE";
        case 3:
            return "STAR";
        default:
            return "SMALL";
    }
}

// Maps the player's live PowerState enum to the 0-3 cheat slot index.
// SMALL_STAR→0 (treated as SMALL), BIG_STAR→3 (STAR slot).
static int PowerStateToIndex(Mario::PowerState ps) {
    switch (ps) {
        case Mario::PowerState::BIG:        return 1;
        case Mario::PowerState::FIRE:       return 2;
        case Mario::PowerState::BIG_STAR:   return 3;
        case Mario::PowerState::SMALL_STAR: return 0;
        default:                            return 0;  // SMALL
    }
}

// ============================================================================
// OnEnter — seed m_PowerStateIndex from the player's actual current state.
// Called once when App transitions INTO ESC_MENU, before the first Update().
// ============================================================================
void ESCMenuSceneHandler::OnEnter(App& app) {
    auto& player = app.GetPlayer();
    if (player) {
        m_PowerStateIndex =
            PowerStateToIndex(player->GetState().GetPowerState());
    }
}

// ============================================================================
// Update
// ============================================================================
void ESCMenuSceneHandler::Update(App& app) {
    int& sel = app.GetESCMenuSelection();

    if (Util::Input::IsKeyDown(Util::Keycode::UP))
        sel = (sel - 1 + MENU_ITEM_COUNT) % MENU_ITEM_COUNT;
    if (Util::Input::IsKeyDown(Util::Keycode::DOWN))
        sel = (sel + 1) % MENU_ITEM_COUNT;

    if (Util::Input::IsKeyDown(Util::Keycode::RETURN)) {
        switch (sel) {
            case 0:  // Resume
                app.TransitionTo(App::State::PLAYING);
                app.PlayCurrentBGM();
                LOG_INFO("Resuming game");
                break;
            case 1:  // Jump to 1-1
                app.GetGameState().SetLevel(1, 1);
                app.TransitionTo(App::State::LOADING);
                LOG_INFO("Jumping to World 1-1");
                break;
            case 2:  // Jump to 1-2
                app.GetGameState().SetLevel(1, 2);
                app.TransitionTo(App::State::LOADING);
                LOG_INFO("Jumping to World 1-2");
                break;
            case 3:  // Jump to 8-4
                app.GetGameState().SetLevel(8, 4);
                app.TransitionTo(App::State::LOADING);
                LOG_INFO("Jumping to World 8-4");
                break;
            case 4: {  // Power cheat: cycle and apply
                m_PowerStateIndex = (m_PowerStateIndex + 1) % 4;
                auto& player = app.GetPlayer();
                if (player) {
                    player->GetState().ForceApplyPowerState(m_PowerStateIndex);
                    LOG_INFO("Cheat: Mario power -> {}",
                             GetPowerStateName(m_PowerStateIndex));
                }
                break;
            }
        }
    }

    if (Util::Input::IsKeyDown(Util::Keycode::ESCAPE)) {
        app.TransitionTo(App::State::PLAYING);
        app.PlayCurrentBGM();
        LOG_INFO("ESC pressed again - resuming game");
    }
}

// ============================================================================
// OnRender
// ============================================================================
void ESCMenuSceneHandler::OnRender(App& app) {
    app.GetRenderer().Update();
    app.GetUIManager().Update(Mario::UIManager::State::ESC_MENU,
                              app.GetESCMenuSelection(),
                              GetPowerStateName(m_PowerStateIndex));
}

}  // namespace Mario
