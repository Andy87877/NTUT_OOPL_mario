/**
 * @file TitleSceneHandler.hpp
 * @brief Title screen scene handler (START or TITLE state).
 *        Displays intro screen, waits for user input to start game.
 * @inheritance ISceneHandler
 */
#ifndef MARIO_TITLE_SCENE_HANDLER_HPP
#define MARIO_TITLE_SCENE_HANDLER_HPP

#include <memory>

#include "Mario/ISceneHandler.hpp"
#include "Util/Image.hpp"

namespace Mario {

/**
 * Manages the title/intro scene.
 * Displays "Super Mario Bros" logo, waits for START key press (or similar).
 * Then transitions to LOADING scene.
 */
class TitleSceneHandler : public ISceneHandler {
   public:
    TitleSceneHandler();
    virtual ~TitleSceneHandler() = default;

    void OnEnter() override;
    bool Update() override;
    void OnExit() override;

    const char* GetNextSceneName() const override;
    const char* GetName() const override { return "TitleScene"; }

   private:
    std::shared_ptr<Util::Image> m_TitleImage;
    int m_TimeCounter = 0;
    const char* m_NextScene = nullptr;
    bool m_Started = false;
};

}  // namespace Mario

#endif  // MARIO_TITLE_SCENE_HANDLER_HPP
