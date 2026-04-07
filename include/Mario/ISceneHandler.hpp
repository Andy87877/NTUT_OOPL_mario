/**
 * @file ISceneHandler.hpp
 * @brief Interface for scene handlers implementing the strategy pattern.
 *        Each handler manages a specific game scene (Title, Loading, Playing,
 * etc.)
 * @inheritance None (interface)
 */
#ifndef MARIO_I_SCENE_HANDLER_HPP
#define MARIO_I_SCENE_HANDLER_HPP

namespace Mario {

/**
 * Abstract interface for scene handlers.
 * Allows different scene types to implement their own update & render logic.
 */
class ISceneHandler {
   public:
    virtual ~ISceneHandler() = default;

    /**
     * Called once when the scene is activated.
     */
    virtual void OnEnter() = 0;

    /**
     * Called every frame to update scene logic.
     * @return false if scene should transition to next, true to keep running
     */
    virtual bool Update() = 0;

    /**
     * Called once when the scene is deactivated.
     */
    virtual void OnExit() = 0;

    /**
     * Get the next scene name for transition.
     * Only meaningful after Update() returns false.
     */
    virtual const char* GetNextSceneName() const = 0;

    /**
     * Get this scene's name for identification.
     */
    virtual const char* GetName() const = 0;
};

}  // namespace Mario

#endif  // MARIO_I_SCENE_HANDLER_HPP
