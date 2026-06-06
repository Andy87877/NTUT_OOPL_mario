/**
 * @file ESCMenuItems.cpp
 * @brief Concrete implementations of pause menu options (Command Pattern).
 * @inheritance IESCMenuItem -> ResumeMenuItem, LevelWarpMenuItem, PowerCheatMenuItem, CheatToggleMenuItem
 */
#include "Mario/UI/ESCMenuItems.hpp"

#include "App.hpp"
#include "Mario/Player/PlayerState.hpp"
#include "Mario/Scenes/ESCMenuSceneHandler.hpp"
#include "Util/Logger.hpp"

namespace Mario {

// ============================================================================
// Static Helpers for Power Cheat
// ============================================================================
static const char* GetPowerStateName(int idx) {
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

static int PowerStateToIndex(Mario::PowerState ps) {
    switch (ps) {
        case Mario::PowerState::BIG:
            return 1;
        case Mario::PowerState::FIRE:
            return 2;
        case Mario::PowerState::BIG_STAR:
            return 3;
        case Mario::PowerState::SMALL_STAR:
            return 0;
        default:
            return 0;  // SMALL
    }
}

// ============================================================================
// ResumeMenuItem
// ============================================================================
std::string ResumeMenuItem::GetDisplayText(App&) const {
    return "RESUME";
}

std::string ResumeMenuItem::GetDescriptionText() const {
    return "> RESUME GAME AND RETURN TO PLAYING";
}

void ResumeMenuItem::Execute(App& app, ESCMenuSceneHandler&) {
    app.TransitionTo(App::State::PLAYING);
    app.PlayCurrentBGM();
    LOG_INFO("Resuming game");
}

// ============================================================================
// LevelWarpMenuItem
// ============================================================================
LevelWarpMenuItem::LevelWarpMenuItem(int world, int level)
    : m_World(world), m_Level(level) {}

std::string LevelWarpMenuItem::GetDisplayText(App&) const {
    return std::to_string(m_World) + "-" + std::to_string(m_Level);
}

std::string LevelWarpMenuItem::GetDescriptionText() const {
    return "> WARP IMMEDIATELY TO WORLD " + std::to_string(m_World) + "-" + std::to_string(m_Level);
}

void LevelWarpMenuItem::Execute(App& app, ESCMenuSceneHandler&) {
    app.GetGameState().SetLevel(m_World, m_Level);
    app.TransitionTo(App::State::LOADING);
    LOG_INFO("Jumping to World {}-{}", m_World, m_Level);
}

// ============================================================================
// PowerCheatMenuItem
// ============================================================================
std::string PowerCheatMenuItem::GetDisplayText(App& app) const {
    auto& player = app.GetPlayer();
    if (player) {
        int index = PowerStateToIndex(player->GetState().GetPowerState());
        return "POWER: " + std::string(GetPowerStateName(index));
    }
    return "POWER: SMALL";
}

std::string PowerCheatMenuItem::GetDescriptionText() const {
    return "> CYCLE MARIO'S POWER LEVEL STATE";
}

void PowerCheatMenuItem::Execute(App& app, ESCMenuSceneHandler&) {
    auto& player = app.GetPlayer();
    if (player) {
        int index = PowerStateToIndex(player->GetState().GetPowerState());
        index = (index + 1) % 4;
        player->GetState().ForceApplyPowerState(index);
        LOG_INFO("Cheat: Mario power -> {}", GetPowerStateName(index));
    }
}

// ============================================================================
// CheatToggleMenuItem
// ============================================================================
std::string CheatToggleMenuItem::GetDisplayText(App& app) const {
    return std::string("CHEAT: ") + (app.GetGameState().IsCheatModeActive() ? "ON" : "OFF");
}

std::string CheatToggleMenuItem::GetDescriptionText() const {
    return "> TOGGLE INFINITE LIVES & STAR CHEAT";
}

void CheatToggleMenuItem::Execute(App& app, ESCMenuSceneHandler&) {
    auto& gs = app.GetGameState();
    gs.SetCheatModeActive(!gs.IsCheatModeActive());
    LOG_INFO("Cheat Mode toggled: {}", gs.IsCheatModeActive() ? "ON" : "OFF");
}

// ============================================================================
// ControlsESCMenuItem
// ============================================================================
std::string ControlsESCMenuItem::GetDisplayText(App&) const {
    return "CONTROLS";
}

std::string ControlsESCMenuItem::GetDescriptionText() const {
    return "> VIEW DETAILED KEYBOARD CONTROLS GUIDE";
}

void ControlsESCMenuItem::Execute(App&, ESCMenuSceneHandler& handler) {
    handler.SetSubState(ESCMenuSceneHandler::SubState::CONTROLS);
    LOG_INFO("Opening controls instruction guide from Pause Menu");
}

}  // namespace Mario
