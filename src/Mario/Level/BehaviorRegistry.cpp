/**
 * @file BehaviorRegistry.cpp
 * @brief Implementation of BehaviorRegistry mapping.
 * @inheritance None
 */
#include "Mario/Level/BehaviorRegistry.hpp"

#include "Mario/Behaviors/BowserBehavior.hpp"
#include "Mario/Behaviors/CastleFireSpawnerBehavior.hpp"
#include "Mario/Behaviors/DefaultEntityBehavior.hpp"
#include "Mario/Behaviors/FireballBehavior.hpp"
#include "Mario/Behaviors/GoombaBehavior.hpp"
#include "Mario/Behaviors/ItemBehaviors.hpp"
#include "Mario/Behaviors/KoopaFamily.hpp"
#include "Mario/Behaviors/ParticleDebris.hpp"
#include "Mario/Behaviors/PiranhaPlantBehavior.hpp"
#include "Mario/Behaviors/PodobooBehavior.hpp"
#include "Mario/Behaviors/StaticEntityBehaviors.hpp"

namespace Mario {

std::unique_ptr<IEntityBehavior> BehaviorRegistry::Create(EntityType type, const EntityDef& def) {
    static std::unordered_map<EntityType, Creator> s_Registry;
    static bool s_Initialized = false;
    if (!s_Initialized) {
        InitializeRegistry(s_Registry);
        s_Initialized = true;
    }

    auto it = s_Registry.find(type);
    if (it != s_Registry.end()) {
        return it->second(def);
    }
    return std::make_unique<DefaultEntityBehavior>();
}

void BehaviorRegistry::InitializeRegistry(std::unordered_map<EntityType, Creator>& map) {
    map[EntityType::GOOMBA] = [](const EntityDef&) {
        return std::make_unique<GoombaBehavior>();
    };
    map[EntityType::KOOPA_TROOPA] = [](const EntityDef& def) {
        return std::make_unique<KoopaBehavior>(
            def.name == "Koopa" ? KoopaBehavior::KoopaType::RED_TROOPA
                                : KoopaBehavior::KoopaType::TROOPA);
    };
    map[EntityType::PARAKOOPA] = [](const EntityDef&) {
        return std::make_unique<ParaKoopaBehavior>();
    };
    map[EntityType::KOOPA_SHELL] = [](const EntityDef&) {
        return std::make_unique<KoopaBehavior>(KoopaBehavior::KoopaType::SHELL);
    };
    map[EntityType::AXE_KOOPA] = [](const EntityDef&) {
        return std::make_unique<AxeKoopaBehavior>();
    };
    map[EntityType::BOWSER] = [](const EntityDef&) {
        return std::make_unique<BowserBehavior>();
    };
    map[EntityType::CASTLE_FIRE_SPAWNER] = [](const EntityDef&) {
        return std::make_unique<CastleFireSpawnerBehavior>();
    };
    map[EntityType::FIRE] = [](const EntityDef& def) {
        return std::make_unique<FireballBehavior>(
            def.isEnemy ? FireballBehavior::FireballType::BOWSER
                        : FireballBehavior::FireballType::PLAYER);
    };
    map[EntityType::PRINCESS] = [](const EntityDef&) {
        return std::make_unique<PrincessBehavior>();
    };
    map[EntityType::MUSHROOM] = [](const EntityDef&) {
        return std::make_unique<MushroomBehavior>();
    };
    map[EntityType::FIRE_FLOWER] = [](const EntityDef&) {
        return std::make_unique<FireFlowerBehavior>();
    };
    map[EntityType::STAR] = [](const EntityDef&) {
        return std::make_unique<StarBehavior>();
    };
    map[EntityType::ONE_UP] = [](const EntityDef&) {
        return std::make_unique<OneUpBehavior>();
    };
    map[EntityType::COIN] = [](const EntityDef&) {
        return std::make_unique<CoinBehavior>();
    };
    map[EntityType::FLAG] = [](const EntityDef&) {
        return std::make_unique<FlagBehavior>();
    };
    map[EntityType::PARTICLE_DEBRIS] = [](const EntityDef&) {
        return std::make_unique<ParticleDebris>();
    };
    map[EntityType::AXE] = [](const EntityDef&) {
        return std::make_unique<AxeBehavior>();
    };
    map[EntityType::AXE_PROJECTILE] = [](const EntityDef&) {
        return std::make_unique<AxeProjectileBehavior>();
    };
    map[EntityType::PIRANHA_PLANT] = [](const EntityDef&) {
        return std::make_unique<PiranhaPlantBehavior>();
    };
    map[EntityType::PODOBOO] = [](const EntityDef&) {
        return std::make_unique<PodobooBehavior>();
    };
}

} // namespace Mario
