/**
 * @file EntityDef.hpp
 * @brief Data definition struct for entities, parsed from EntityList.csv.
 *        Each row in the CSV maps to one EntityDef instance.
 * @inheritance None (pure data struct)
 */
#ifndef MARIO_ENTITY_DEF_HPP
#define MARIO_ENTITY_DEF_HPP

#include <string>

namespace Mario {

/**
 * Entity type enumeration for behavior configuration.
 * Each entity name maps to a Type value loaded from EntityList.csv.
 */
enum class EntityType {
    GOOMBA,
    KOOPA_TROOPA,
    PARAKOOPA,
    KOOPA_SHELL,
    AXE_KOOPA,
    BOWSER,
    FIRE,
    PRINCESS,
    MUSHROOM,
    FIRE_FLOWER,
    STAR,
    ONE_UP,
    COIN,
    FLAG,
    PARTICLE_DEBRIS,
    AXE,
    AXE_PROJECTILE,
    PIRANHA_PLANT,
    PODOBOO,  // Lava Bubble — jumps from lava, immortal
    CASTLE_FIRE_SPAWNER,
    UNKNOWN
};

/**
 * Holds all properties for one entity type, loaded from EntityList.csv.
 * CSV Format per row:
 *   id, name, type, isPowerUp, isEnemy, isCoin, powerUpState, isStatic,
 *   isBounce, fromBlock, scoreWorth, isAnimated, animFrames,
 *   doesJump, doesCollide, oneLoop, animBuffer, squishable, koopaSquash
 */
struct EntityDef {
    int id = -1;
    std::string name;
    EntityType type = EntityType::UNKNOWN;
    bool isPowerUp = false;
    bool isEnemy = false;
    bool isCoin = false;
    int powerUpState = 0;
    bool isStatic = false;
    bool isBounce = false;
    bool fromBlock = false;
    int scoreWorth = 0;
    bool isAnimated = false;
    int animFrames = 0;
    bool doesJump = false;
    bool doesCollide = false;
    bool oneLoop = false;
    int animBuffer = 1;
    bool squishable = false;
    bool koopaSquash = false;

    // Rendering override: target sprite-space pixel width used to compute the
    // draw scale.  Set by EntityFactory::SpawnEntity() based on entity type
    // and level context (e.g. 8-4 enemy scaling).
    // 0.0f = no override — use the default GameConfig::DRAW_SCALE.
    // This field keeps all level/name-specific scaling knowledge inside the
    // Factory (OCP): Entity.cpp reads the value and applies it blindly.
    float renderTargetWidth = 0.0f;
};

/**
 * Holds all properties for one block type, loaded from IDList.csv.
 * CSV Format per row:
 *   id, name, solid, breakable, background, isGoal, contentID,
 *   r, g, b, isContainer, contents, hitSpriteName,
 *   animated, animationFrames, bounceBack, spawner, spawnEntity
 */
struct BlockDef {
    int id = -1;
    std::string name;
    bool solid = false;
    bool breakable = false;
    bool background = false;
    bool isGoal = false;
    int contentID = 0;
    int r = 0, g = 0, b = 0;  // Background color (RGB)
    bool isContainer = false;
    std::string contents;       // Entity to spawn when hit
    std::string hitSpriteName;  // Sprite after being hit
    bool animated = false;
    int animationFrames = 0;
    bool bounceBack = false;
    bool spawner = false;
    std::string spawnEntity;  // Entity spawned on activation
};

}  // namespace Mario

#endif  // MARIO_ENTITY_DEF_HPP
