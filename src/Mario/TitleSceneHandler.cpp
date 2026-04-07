/**
 * @file TitleSceneHandler.cpp
 * @brief Implementation of title scene handler.
 * @inheritance ISceneHandler <- TitleSceneHandler
 */
#include "Mario/TitleSceneHandler.hpp"

#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include "Util/Logger.hpp"

namespace Mario {

TitleSceneHandler::TitleSceneHandler() : m_TimeCounter(0) {}

void TitleSceneHandler::OnEnter() {
    LOG_INFO("Entered Title Scene");
    // Load title image (if available)
    // m_TitleImage = std::make_shared<Util::Image>("Resources/title.png");
    m_TimeCounter = 0;
    m_Started = false;
}

bool TitleSceneHandler::Update() {
    m_TimeCounter++;

    // Wait for input to start
    if (Util::Input::IsKeyPressed(Util::Keycode::RETURN) ||
        Util::Input::IsKeyPressed(Util::Keycode::SPACE)) {
        if (!m_Started) {
            m_Started = true;
            m_NextScene = "LoadingScene";
            return false;  // Signal transition
        }
    }

    return true;  // Keep running
}

void TitleSceneHandler::OnExit() { LOG_INFO("Exited Title Scene"); }

const char* TitleSceneHandler::GetNextSceneName() const {
    return m_NextScene ? m_NextScene : "LoadingScene";
}

}  // namespace Mario
