/**
 * @file ISceneHandler.hpp
 * @brief Interface for game-state scene handlers (State Pattern).
 *        Each concrete handler owns the per-frame update AND render logic for
 *        one App::State, receiving the full App context.
 *
 *        Adding a new game state requires:
 *          1. A new ISceneHandler subclass (.hpp + .cpp)
 *          2. One case in App::CreateSceneHandler()
 *          3. One entry in App::State enum
 *        Zero changes to App::Update() or any other file.
 *
 * @inheritance None (pure interface)
 *
 * Inheritance tree:
 *   ISceneHandler
 *   ├── TitleSceneHandler
 *   ├── LoadingSceneHandler
 *   ├── PlayingSceneHandler
 *   ├── FlagpoleSceneHandler
 *   ├── PipeWarpSceneHandler
 *   ├── AxeSequenceSceneHandler
 *   ├── DeathSceneHandler
 *   ├── GameOverSceneHandler
 *   ├── GameWonSceneHandler
 *   └── ESCMenuSceneHandler
 */
#ifndef MARIO_I_SCENE_HANDLER_HPP
#define MARIO_I_SCENE_HANDLER_HPP

// App lives in the global namespace; forward-declare here to avoid a circular
// include (App.hpp includes this header, so we cannot include App.hpp here).
class App;

namespace Mario {

/**
 * Pure-virtual strategy interface for App game-state handlers.
 *
 * Lifecycle per frame:
 *   1. App::Update() calls Update(*this) — game logic
 *   2. App::Update() calls OnRender(*this) — drawing
 *
 * Each concrete handler implements both, owning all logic that is
 * specific to that game state.  No central switch-case needed in App.
 */
class ISceneHandler {
   public:
    virtual ~ISceneHandler() = default;

    /** Called once when this state becomes active. Default: no-op. */
    virtual void OnEnter(App& /*app*/) {}

    /** Per-frame update: game logic for this state. Pure virtual. */
    virtual void Update(App& app) = 0;

    /**
     * Per-frame render: set background color, call Renderer, update UI.
     * Default implementation just calls app.GetRenderer().Update().
     * Override to set the correct background and UIManager state for this
     * scene.
     */
    virtual void OnRender(App& app);

    /** Called once when transitioning away from this state. Default: no-op. */
    virtual void OnExit(App& /*app*/) {}

    /** Debug/logging name for this handler. */
    virtual const char* GetName() const = 0;
};

}  // namespace Mario

#endif  // MARIO_I_SCENE_HANDLER_HPP
