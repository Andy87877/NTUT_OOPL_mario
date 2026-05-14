/**
 * @file KoopaFamily.hpp
 * @brief All Koopa-family enemy behaviors in one header.
 *        KoopaBehavior, AxeKoopaBehavior, and ParaKoopaBehavior share the same
 *        base interface and represent the three Koopa-type variants.
 *        Consolidated because all three are exclusively Koopa sub-types with
 *        no consumers outside the entity sub-system.
 * @inheritance IEntityBehavior <- KoopaBehavior
 *              IEntityBehavior <- AxeKoopaBehavior
 *              IEntityBehavior <- ParaKoopaBehavior
 */
#ifndef MARIO_KOOPA_FAMILY_HPP
#define MARIO_KOOPA_FAMILY_HPP

#include <functional>
#include <memory>

#include "Mario/Behaviors/IEntityBehavior.hpp"

namespace Mario {

// ============================================================================
// KoopaBehavior — standard Koopa Troopa + shell form
// ============================================================================
/**
 * Handles patrol movement for living KoopaTroopa and passive shell.
 * Shell spawning is delegated to App::CheckPlayerEntityCollision.
 * @inheritance IEntityBehavior <- KoopaBehavior
 */
class KoopaBehavior : public IEntityBehavior {
   public:
    enum class KoopaType {
        TROOPA,  // Living Koopa (patrol movement)
        SHELL,   // Shell form (passive, moved only via external velocity)
    };

    explicit KoopaBehavior(KoopaType type = KoopaType::TROOPA);
    ~KoopaBehavior() override = default;

    void Update(EntityState& state, const Level& level, const Player& player,
                int gameTimer) override;
    bool OnPlayerCollision(EntityState& state, Player& player,
                           bool isFromAbove) override;
    std::unique_ptr<IEntityBehavior> Clone() const override;
    const char* GetName() const override { return "KoopaBehavior"; }

   private:
    KoopaType m_Type;
    int m_DirectionChangeCounter = 0;
    int m_LastDirectionChangeFrame = -100;
};

// ============================================================================
// AxeKoopaBehavior — walking Koopa that throws axes periodically
// ============================================================================
/**
 * Patrol + periodic axe-throw attack via a spawn callback.
 * Immune to stomp (like Bowser). Only defeated by axe trap.
 * @inheritance IEntityBehavior <- AxeKoopaBehavior
 */
class AxeKoopaBehavior : public IEntityBehavior {
   public:
    using AxeSpawnCallback = std::function<void(float x, float y)>;

    AxeKoopaBehavior() = default;
    ~AxeKoopaBehavior() override = default;

    /** Attach the axe-spawn callback from App. */
    void SetAxeSpawnCallback(AxeSpawnCallback callback) {
        m_AxeSpawnCallback = std::move(callback);
    }

    void Update(EntityState& state, const Level& level, const Player& player,
                int gameTimer) override;
    bool OnPlayerCollision(EntityState& state, Player& player,
                           bool isFromAbove) override;
    std::unique_ptr<IEntityBehavior> Clone() const override;
    const char* GetName() const override { return "AxeKoopaBehavior"; }

   private:
    static constexpr float WALK_SPEED = 0.5f;
    static constexpr int AXE_THROW_INTERVAL = 150;  // ~2.5 s at 60 FPS

    int m_ThrowTimer = 0;
    AxeSpawnCallback m_AxeSpawnCallback;
};

// ============================================================================
// ParaKoopaBehavior — flying Koopa with sine-wave oscillation
// ============================================================================
/**
 * Floats vertically via sine wave; loses wings and falls when stomped.
 * @inheritance IEntityBehavior <- ParaKoopaBehavior
 */
class ParaKoopaBehavior : public IEntityBehavior {
   public:
    ParaKoopaBehavior() = default;
    ~ParaKoopaBehavior() override = default;

    void Update(EntityState& state, const Level& level, const Player& player,
                int gameTimer) override;
    bool OnPlayerCollision(EntityState& state, Player& player,
                           bool isFromAbove) override;
    std::unique_ptr<IEntityBehavior> Clone() const override;
    const char* GetName() const override { return "ParaKoopaBehavior"; }

   private:
    static constexpr float FLOAT_AMPLITUDE = 1.5f;  // Oscillation distance (px)
    static constexpr float FLOAT_FREQUENCY = 2.0f;  // Oscillation rate (rad/s)

    float m_FloatPhase = 0.0f;
    float m_OriginalY = 0.0f;
    bool m_IsFlying = true;
};

}  // namespace Mario

#endif  // MARIO_KOOPA_FAMILY_HPP
