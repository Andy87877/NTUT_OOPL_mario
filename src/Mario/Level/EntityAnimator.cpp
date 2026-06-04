/**
 * @file EntityAnimator.cpp
 * @brief Implementation of EntityAnimator (SRP Decoupling).
 *        Resolves the correct sprite path based on EntityState and EntityDef.
 * @inheritance None
 */
#include "Mario/Level/EntityAnimator.hpp"

#include "Mario/Level/EntityState.hpp"
#include "Mario/Level/EntityDef.hpp"
#include "Mario/Core/SpritePathResolver.hpp"

namespace Mario {

std::string EntityAnimator::GetSpritePath(const EntityState& state, const EntityDef& def,
                                          const std::string& levelName) const {
    const std::string& name = state.GetName();

    // Squished or dead Koopa family members use the Shell sprite
    if (state.IsSquished() || (state.IsDead() && state.IsKoopaSquash())) {
        if (state.IsKoopaSquash()) {
            // Koopa/ParaKoopa shells use KoopaShell sprite
            return SpritePathResolver::GetEntitySpritePath("KoopaShell", -1,
                                                           levelName);
        }
        return SpritePathResolver::GetEntitySpritePath(name + "Squish", -1,
                                                       levelName);
    }

    // Special handling: KoopaTroopaShell uses KoopaShell sprite
    // (KoopaTroopaShell is a dynamic entity created when KoopaTroopa is
    // stomped)
    std::string displayName = name;
    if (name == "KoopaTroopaShell") {
        displayName = "KoopaShell";
    }

    // Animated entities: name + frame (1-indexed in C#)
    if (def.isAnimated) {
        int frame = state.GetAnimFrame() + 1;  // C# uses 1-indexed frames
        return SpritePathResolver::GetEntitySpritePath(displayName, frame,
                                                       levelName);
    }

    // Static/single-frame entities
    return SpritePathResolver::GetEntitySpritePath(displayName, -1,
                                                   levelName);
}

}  // namespace Mario
