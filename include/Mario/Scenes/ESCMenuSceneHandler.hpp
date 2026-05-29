/**
 * @file ESCMenuSceneHandler.hpp
 * @brief Pause/ESC menu scene handler (App::State::ESC_MENU).
 *        Navigate with UP/DOWN, confirm with ENTER, ESC resumes.
 *        Menu items (0-3): RESUME, 1-1, 1-2, 8-4.
 *        Item 4 is the "POWER" cheat: cycles Mario's power state
 *        (SMALL -> BIG -> FIRE -> STAR -> SMALL) and applies it instantly.
 * @inheritance ISceneHandler <- ESCMenuSceneHandler
 */
#ifndef MARIO_ESC_MENU_SCENE_HANDLER_HPP
#define MARIO_ESC_MENU_SCENE_HANDLER_HPP

#include "Mario/Scenes/ISceneHandler.hpp"

namespace Mario {

class ESCMenuSceneHandler : public ISceneHandler {
   public:
    ESCMenuSceneHandler() = default;

    /** Seed m_PowerStateIndex from the player's current power state. */
    void OnEnter(App& app) override;
    void Update(App& app) override;
    void OnRender(App& app) override;
    const char* GetName() const override { return "ESCMenuScene"; }

    /**
     * Total number of items in the ESC menu.
     * 0=RESUME  1=1-1  2=1-2  3=8-4  4=POWER CHEAT  5=CHEAT MODE
     */
    static constexpr int MENU_ITEM_COUNT = 6;

    /**
     * Returns the display name for the cheat power-cycle slot.
     * @param idx  0=SMALL, 1=BIG, 2=FIRE, 3=STAR
     */
    static const char* GetPowerStateName(int idx);

   private:
    // Which power state the cheat item currently shows (0-3).
    int m_PowerStateIndex = 0;
};

}  // namespace Mario

#endif  // MARIO_ESC_MENU_SCENE_HANDLER_HPP
