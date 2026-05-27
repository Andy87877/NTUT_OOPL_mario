/**
 * @file ILevelService.hpp
 * @brief Abstract interface defining the Level Service for loading and managing
 *        active level states, entities, players, and backgrounds, decoupling
 *        these details from the main App class.
 *
 * @inheritance None (pure interface)
 */
#ifndef MARIO_I_LEVEL_SERVICE_HPP
#define MARIO_I_LEVEL_SERVICE_HPP

#include <memory>
#include <string>
#include <vector>

// Forward declarations
namespace Mario {
class Level;
class Player;
class Entity;
}  // namespace Mario
class App;

namespace Mario {

/**
 * Abstract interface for the Level Service.
 * Follows the Dependency Inversion Principle (DIP).
 */
class ILevelService {
   public:
    virtual ~ILevelService() = default;

    /** Load a level by name and setup player, entities, and renderer. */
    virtual void LoadLevel(App& app, const std::string& levelName) = 0;

    /** Start the gameplay of the loaded level (unhide world, start timers, play music). */
    virtual void StartLevel(App& app) = 0;

    /** Play the correct background music based on the level context and time remaining. */
    virtual void PlayCurrentBGM(App& app) = 0;

    /** Add dynamically spawned entity to both level tracking and renderer. */
    virtual void AddEntityToGame(App& app, std::shared_ptr<Entity> entity) = 0;

    /** Transition game state to loading the next level. */
    virtual void AdvanceToNextLevel(App& app) = 0;

    /** Query whether the current level utilizes an underground or castle background. */
    virtual bool IsUnderground(App& app) const = 0;

    /** Clear background color to match current level environment. */
    virtual void ApplyBackground(App& app) = 0;

    /** Clear background color to specific environment (black vs sky-blue). */
    virtual void ApplyBackground(App& app, bool isUnderground) = 0;

    // -- Subsystem Accessors --
    virtual std::shared_ptr<Level>& GetLevel() = 0;
    virtual std::shared_ptr<Player>& GetPlayer() = 0;
    virtual std::vector<std::shared_ptr<Entity>>& GetEntities() = 0;
    virtual std::shared_ptr<Entity>& GetFlagEntity() = 0;
    virtual const std::string& GetCurrentLevelName() const = 0;

    // -- Primitive mutable accessors for handlers --
    virtual bool& GetLoading() = 0;
    virtual int& GetLoadTimer() = 0;
    virtual int& GetDeathTimer() = 0;
    virtual int& GetESCMenuSelection() = 0;
};

}  // namespace Mario

#endif  // MARIO_I_LEVEL_SERVICE_HPP
