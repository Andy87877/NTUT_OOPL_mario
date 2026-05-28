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
        TROOPA,      // Living green Koopa (walks off cliffs)
        RED_TROOPA,  // Living red Koopa (turns around at ledges)
        SHELL,       // Shell form (passive, moved only via external velocity)
    };

    explicit KoopaBehavior(KoopaType type = KoopaType::TROOPA);
    ~KoopaBehavior() override = default;

    void Update(EntityState& state, const Level& level, const Player& player,
                int gameTimer) override;
    bool OnPlayerCollision(EntityState& state, Player& player,
                           bool isFromAbove) override;
    std::unique_ptr<IEntityBehavior> Clone() const override;
    const char* GetName() const override { return "KoopaBehavior"; }
    bool IsShell() const override;
    float GetVisualYOffset(const std::string& levelName) const override;

   private:
    KoopaType m_Type;
    int m_DirectionChangeCounter = 0;
    int m_LastDirectionChangeFrame = -100;
};

// ============================================================================
// AxeKoopaBehavior — walking Koopa that throws axes periodically
// ============================================================================
/**
 * Patrol + periodic axe-throw attack via ConsumeSpawnRequest pattern.
 * Immune to stomp (like Bowser). Only defeated by axe trap.
 * @inheritance IEntityBehavior <- AxeKoopaBehavior
 */
class AxeKoopaBehavior : public IEntityBehavior {
   public:
    AxeKoopaBehavior() = default;
    ~AxeKoopaBehavior() override = default;

    void Update(EntityState& state, const Level& level, const Player& player,
                int gameTimer) override;
    bool OnPlayerCollision(EntityState& state, Player& player,
                           bool isFromAbove) override;
    bool ConsumeSpawnRequest(EntityType& outType, float& outX, float& outY,
                             int& outDir) override;
    std::unique_ptr<IEntityBehavior> Clone() const override;
    const char* GetName() const override { return "AxeKoopaBehavior"; }

    /** AxeKoopa spawns enemy axe projectiles (OCP — no EntityType check). */
    bool IsEnemySpawner() const override { return true; }

   private:
    static constexpr float WALK_SPEED = 0.5f;
    static constexpr int AXE_THROW_INTERVAL = 80;  // ~1.3 s at 60 FPS

    int m_ThrowTimer = 0;
    bool m_AxePending = false;
    float m_AxeX = 0.0f;
    float m_AxeY = 0.0f;
    int m_AxeDir = 0;
};

// ============================================================================
// ParaKoopaBehavior — flying Koopa with vertical patrol
// ============================================================================
/**
 * Patrols vertically (up/down); loses wings and falls when stomped.
 * @inheritance IEntityBehavior <- ParaKoopaBehavior
 */
class ParaKoopaBehavior : public IEntityBehavior {
   public:
    enum class Mode {
        FLYING,
        WALKING,
        SHELL,
    };

    ParaKoopaBehavior() = default;
    ~ParaKoopaBehavior() override = default;

    void Update(EntityState& state, const Level& level, const Player& player,
                int gameTimer) override;
    bool OnPlayerCollision(EntityState& state, Player& player,
                           bool isFromAbove) override;
    std::unique_ptr<IEntityBehavior> Clone() const override;
    const char* GetName() const override { return "ParaKoopaBehavior"; }
    bool IsShell() const override;
    float GetVisualYOffset(const std::string& levelName) const override;

   private:
    static constexpr float PATROL_RANGE = 45.0f;  // +/- 1 tile around anchor Y
    static constexpr float PATROL_SPEED = 1.15f;  // px/frame

    float m_AnchorY = 0.0f;
    bool m_AnchorInitialized = false;
    int m_VerticalDir = -1;  // -1 up, +1 down
    Mode m_Mode = Mode::FLYING;
};

}  // namespace Mario

#endif  // MARIO_KOOPA_FAMILY_HPP
