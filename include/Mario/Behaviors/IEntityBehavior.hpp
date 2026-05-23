/**
 * @file IEntityBehavior.hpp
 * @brief Interface for entity behaviors using Strategy pattern.
 *        Allows different behavior implementations (Enemy, PowerUp, etc.)
 *        without modifying Entity class.
 * @inheritance None (interface)
 */
#ifndef MARIO_I_ENTITY_BEHAVIOR_HPP
#define MARIO_I_ENTITY_BEHAVIOR_HPP

#include <memory>

#include "Mario/EntityDef.hpp"  // EntityType

namespace Mario {

// Forward declaration
class Entity;
class EntityState;
class Level;
class Player;

/**
 * Abstract behavior interface for entities.
 * Each behavior type (Enemy, PowerUp, Coin, etc.) implements this.
 */
class IEntityBehavior {
   public:
    virtual ~IEntityBehavior() = default;

    /**
     * Update entity behavior logic each frame.
     * Called by Entity during game loop.
     * @param state The entity's model data
     * @param level Level reference for collision/block data
     * @param player Player reference for proximity checks
     * @param gameTimer Current game timer tick
     */
    virtual void Update(EntityState& state, const Level& level,
                        const Player& player, int gameTimer) = 0;

    /**
     * Handle collision with player.
     * Return true if special collision was processed (e.g., defeat enemy,
     * pickup power-up).
     * @param state Entity state
     * @param player Player reference
     * @param isFromAbove True if player hit from above (jumping on enemy)
     * @return True if collision was special (consumed)
     */
    virtual bool OnPlayerCollision(EntityState& state, Player& player,
                                   bool isFromAbove) = 0;

    /**
     * Clone this behavior for a new entity instance.
     * @return New behavior instance copy
     */
    virtual std::unique_ptr<IEntityBehavior> Clone() const = 0;

    /**
     * Get behavior name for debugging.
     * @return Name of this behavior type (e.g., "GoombaBehavior")
     */
    virtual const char* GetName() const = 0;

    /**
     * Whether this behavior must be updated even when off-screen.
     * Default: false. Override to return true for short-lived particles
     * (ParticleDebris) so they always advance and clean up correctly.
     * Eliminates the string-find hack in PlayingSceneHandler.
     */
    virtual bool AlwaysUpdate() const { return false; }

    /**
     * Called exactly once immediately after the entity is spawned into the
     * world by SpawnBrickDebris (or any factory that needs to pass initial
     * velocity). Default is a no-op; ParticleDebris overrides to store vx/vy.
     * Eliminates the dynamic_cast<ParticleDebris*> in PlayingSceneHandler.
     */
    virtual void OnSpawned(float vx, float vy) {
        (void)vx;
        (void)vy;
    }

    /**
     * Handle a fireball hit on this entity.
     * Default behavior: return false → entity will be deleted by caller.
     * Override (e.g., BowserBehavior) to use HP system: return true to
     * indicate the behavior handled the hit internally.
     * @param state Entity state to modify
     * @return true if the behavior handled it (do NOT delete entity),
     *         false if the caller should delete the entity normally.
     */
    virtual bool OnFireballHit(EntityState& state) {
        (void)state;
        return false;
    }

    /**
     * Query and consume a pending entity spawn request.
     * BowserBehavior uses this to signal when it wants to shoot a fireball.
     * Default: no spawn pending (returns false).
     * @param outType  EntityType of the entity to spawn
     * @param outX     World X position for the new entity
     * @param outY     World Y position for the new entity
     * @param outDir   Direction for the new entity (0=Left, 1=Right)
     * @return true if there is a spawn request (outType/outX/outY/outDir set),
     *         false if nothing to spawn.
     */
    /**
     * Query and consume a pending entity spawn request.
     * BowserBehavior / AxeKoopaBehavior use this to signal projectile spawns.
     * Uses EntityType directly (type-safe — no raw int cast at the call site).
     * Default: no spawn pending.
     */
    virtual bool ConsumeSpawnRequest(EntityType& outType, float& outX,
                                     float& outY, int& outDir) {
        (void)outType;
        (void)outX;
        (void)outY;
        (void)outDir;
        return false;
    }
};

}  // namespace Mario

#endif  // MARIO_I_ENTITY_BEHAVIOR_HPP
