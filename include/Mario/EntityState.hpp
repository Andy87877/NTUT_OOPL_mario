/**
 * @file EntityState.hpp
 * @brief Entity state data (Model layer for Entity MVC).
 *        Holds position, velocity, animation, and behavior flags.
 *        Ported from C# Entity.cs variables and logic.
 * @inheritance None (pure data Model)
 */
#ifndef MARIO_ENTITY_STATE_HPP
#define MARIO_ENTITY_STATE_HPP

#include <memory>
#include <string>

#include "Mario/Collider.hpp"
#include "Mario/EnemyDeathAnimation.hpp"
#include "Mario/GameConfig.hpp"

namespace Mario {

/**
 * Entity state model — mirrors the C# Entity class data fields.
 * Goomba, KoopaTroopa, Mushroom, Star, etc. all use this.
 */
class EntityState {
   public:
    EntityState();

    void Init(const std::string& name, float worldX, float worldY,
              int direction, bool isEnemy, bool isPowerUp, bool isCoin,
              bool isStatic, bool doesCollide, bool squishable,
              bool koopaSquash, bool doesJump, bool isBounce, int scoreWorth,
              bool isAnimated, int animFrames, int animBuffer, bool oneLoop,
              bool fromBlock, int powerUpState);

    // -- Per-frame update --
    void Tick();

    // -- Getters --
    float GetX() const { return m_PosX; }
    float GetY() const { return m_PosY; }
    float GetVelX() const { return m_VelX; }
    double GetVelY() const { return m_VelY; }
    double GetFallHeight() const { return m_FallHeight; }
    int GetWidth() const { return m_SizeX; }
    int GetHeight() const { return m_SizeY; }
    const std::string& GetName() const { return m_Name; }
    int GetDirection() const { return m_Direction; }
    int GetAnimFrame() const { return m_CurrentFrame; }
    int GetScoreWorth() const { return m_ScoreWorth; }
    int GetPowerUpState() const { return m_PowerUpState; }
    bool IsActive() const { return m_Active; }
    bool IsEnemy() const { return m_IsEnemy; }
    bool IsPowerUp() const { return m_IsPowerUp; }
    bool IsCoin() const { return m_IsCoin; }
    bool IsStatic() const { return m_IsStatic; }
    bool IsGrounded() const { return m_IsGrounded; }
    bool IsSquished() const { return m_Squashed; }
    bool IsSquishable() const { return m_Squishable; }
    bool IsKoopaSquash() const { return m_KoopaSquash; }
    bool DoesCollide() const { return m_DoesCollide; }
    bool IsBounce() const { return m_IsBounce; }
    bool IsFromBlock() const { return m_FromBlock; }
    bool IsDeathActive() const {
        return m_DeathAnimation && m_DeathAnimation->IsActive();
    }
    bool IsAnimated() const { return m_IsAnimated; }
    bool IsDead() const { return !m_Active || IsDeathActive(); }
    bool IsInShellMode() const { return m_KoopaSquash && m_Squashed; }
    float GetWorldX() const { return m_PosX; }
    float GetWorldY() const { return m_PosY; }
    AABB GetCollider() const { return GetHitbox(); }

    // -- Setters --
    void SetX(float x) { m_PosX = x; }
    void SetY(float y) { m_PosY = y; }
    void SetWorldX(float x) { m_PosX = x; }
    void SetWorldY(float y) { m_PosY = y; }
    void SetVelX(float vx) { m_VelX = vx; }
    void SetVelY(double vy) { m_VelY = vy; }
    void SetGrounded(bool g) { m_IsGrounded = g; }
    void SetActive(bool a) { m_Active = a; }
    void SetDirection(int d) { m_Direction = d; }
    void SetName(const std::string& name) { m_Name = name; }
    void SetFallHeight(double h) { m_FallHeight = h; }
    void SetSizeX(int sizeX) { m_SizeX = sizeX; }
    void SetSizeY(int sizeY) { m_SizeY = sizeY; }
    void AdvanceAnimationFrame() {
        if (m_IsAnimated && ++m_CurrentFrame >= m_AnimFrames) {
            m_CurrentFrame = 0;
        }
    }

    void SetCollidable(bool collidable) { m_DoesCollide = collidable; }
    void SetGravity(bool gravity) { m_ApplyGravity = gravity; }
    // Temporarily hide entity from rendering while keeping it logically active.
    // Used by PiranhaPlantBehavior when the plant is inside the pipe.
    void SetHidden(bool hidden) { m_Hidden = hidden; }
    bool IsHidden() const { return m_Hidden; }

    // -- Actions --
    void FlipDirection();
    void Squish();
    void TriggerDeath(EnemyDeathCause cause);
    void SetSquashed(bool val) { m_Squashed = val; }
    void SetDeathAnimationStrategy(
        std::unique_ptr<IEnemyDeathAnimation> strategy);
    void KickShell(float speed);
    void Delete();
    void Jump();

    // -- Collision --
    AABB GetHitbox() const;

    // -- Gravity --
    float ApplyGravity();

   private:
    std::string m_Name;
    float m_PosX = 0.0f;
    float m_PosY = 0.0f;
    float m_VelX = 0.0f;
    double m_VelY = 0.0;
    double m_FallHeight = 0.0;
    int m_SizeX = GameConfig::TILE_SIZE;
    int m_SizeY = GameConfig::TILE_SIZE;
    int m_Direction = 1;  // 0=Left, 1=Right, 2=None

    bool m_Active = true;
    bool m_ApplyGravity = true;
    bool m_IsEnemy = false;
    bool m_IsPowerUp = false;
    bool m_IsCoin = false;
    bool m_IsStatic = false;
    bool m_IsGrounded = false;
    bool m_DoesCollide = true;
    bool m_Squishable = false;
    bool m_KoopaSquash = false;
    bool m_DoesJump = false;
    bool m_IsBounce = false;
    bool m_FromBlock = false;
    int m_ScoreWorth = 0;
    int m_PowerUpState = 0;

    // Animation
    bool m_IsAnimated = false;
    int m_AnimFrames = 0;
    int m_CurrentFrame = 0;
    int m_AnimBuffer = 1;
    int m_AnimBufferCount = 0;
    bool m_OneLoop = false;

    // Death/squish
    bool m_Squashed = false;
    std::unique_ptr<IEnemyDeathAnimation> m_DeathAnimation;
    int m_ActiveCounter = 0;
    bool m_Hidden = false;  // Set by PiranhaPlantBehavior to suppress rendering
};

}  // namespace Mario

#endif  // MARIO_ENTITY_STATE_HPP
