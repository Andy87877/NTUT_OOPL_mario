/**
 * @file PlayerState.cpp
 * @brief Implementation of PlayerState (Model layer).
 *        All game logic: physics, power-up transitions, animation key
 * generation. Matches the C# Player.cs logic.
 * @inheritance None (pure Model)
 */
#include "Mario/Player/PlayerState.hpp"
#include "Mario/Player/PlayerForm.hpp"

#include <algorithm>
#include <cmath>

#include "Mario/Services/AudioManager.hpp"
#include "Mario/Core/PhysicsEngine.hpp"
#include "Mario/Services/ServiceLocator.hpp"
#include "Mario/Level/GameStateManager.hpp"

namespace Mario {

PlayerState::PlayerState()
    : m_DeathAnimation(std::make_unique<ClassicPlayerDeathAnimation>()),
      m_Form(std::make_unique<SmallPlayerForm>()) {}

PlayerState::~PlayerState() = default;

void PlayerState::Init(float worldX, float worldY, int startState) {
    m_PosX = worldX;
    m_PosY = worldY;
    m_VelX = 0.0f;
    m_VelY = 0.0;
    m_FallHeight = 0.0;
    m_PowerState = static_cast<PowerState>(startState);
    m_Form = CreatePlayerForm(m_PowerState);
    m_LastJumpPoint = {worldX, worldY};
    m_Grounded = false;
    m_Crouching = false;
    m_MovingRight = false;
    m_MovingLeft = false;
    m_FacingRight = true;
    m_Running = false;
    m_Dead = false;
    m_Controllable = true;
    m_PoleSliding = false;
    m_AnimFrame = 0;
    m_AnimTimer = 0;
    m_InvTimer = -1;
    m_StarTimer = 0;
    m_SpecialActive = false;
    m_MemoryState = PowerState::SMALL;
    m_TransitionTimer = 0;
    m_PrevPowerState = PowerState::SMALL;
    m_CheatStarActive = false;
    m_DeathAnimation = std::make_unique<ClassicPlayerDeathAnimation>();
}

void PlayerState::Tick() {
    // Handle shape transition freeze
    if (m_TransitionTimer > 0) {
        m_TransitionTimer--;
        m_VelX = 0.0f;
        m_VelY = 0.0;
        m_FallHeight = 0.0;
        return; // Freeze movement and physics during transition
    }

    // Update invincibility timer
    if (m_InvTimer > 0) {
        m_InvTimer--;
    }

    // Star timer
    bool cheatActive = false;
    if (ServiceLocator::GetInstance().HasService<GameStateManager>()) {
        auto gs = ServiceLocator::GetInstance().GetService<GameStateManager>();
        if (gs && gs->IsCheatModeActive()) {
            cheatActive = true;
        }
    }

    if (cheatActive) {
        if (m_Form->GetPowerState() != PowerState::SMALL_STAR &&
            m_Form->GetPowerState() != PowerState::BIG_STAR) {
            StartStar();
        }
        if (m_StarTimer < GameConfig::CHEAT_STAR_TIMER_RESET_THRESHOLD) {
            m_StarTimer = GameConfig::CHEAT_STAR_TIMER_MAX; // Keep it high to remain infinite but let it count down for animation cycling
        }
        m_CheatStarActive = true;
    } else if (m_CheatStarActive) {
        // Cheat was just disabled -> immediately end star state
        m_StarTimer = 0;
        m_CheatStarActive = false;
        if (m_Form->GetPowerState() == PowerState::SMALL_STAR ||
            m_Form->GetPowerState() == PowerState::BIG_STAR) {
            m_Form = CreatePlayerForm(m_MemoryState);
            m_PowerState = m_MemoryState;
        }
    }

    if (m_StarTimer > 0) {
        m_StarTimer--;
        if (m_StarTimer <= 0) {
            // Revert from star state
            if (m_Form->GetPowerState() == PowerState::SMALL_STAR ||
                m_Form->GetPowerState() == PowerState::BIG_STAR) {
                m_Form = CreatePlayerForm(m_MemoryState);
                m_PowerState = m_MemoryState;
            }
        }
    }

    // Fire shooting cooldown
    if (m_SpecialActive) {
        m_SpecialCounter++;
        if (m_SpecialCounter >= m_SpecialLength) {
            m_SpecialActive = false;
            m_SpecialCounter = 0;
        }
    }

    // Animation timer (boomerang effect from C# reference)
    m_AnimTimer++;
    if (m_AnimTimer >= m_TargetRate) {
        m_AnimTimer = 0;
        if (!m_BoomerangAnim) {
            if (m_AnimFrame >= m_TotalFrames) {
                m_BoomerangAnim = true;
                m_AnimFrame--;
            } else {
                m_AnimFrame++;
            }
        } else {
            if (m_AnimFrame == 0) {
                m_BoomerangAnim = false;
            } else {
                m_AnimFrame--;
            }
        }
        if (m_AnimFrame < 0) {
            m_AnimFrame = 0;
        }
    }

    // Set totalFrames based on current state (matching C# Update logic)
    if (m_PoleSliding) {
        m_TotalFrames = 0;
        m_AnimFrame = m_Grounded ? 1 : 0;
    } else if (!m_SpecialActive) {
        if (m_Grounded) {
            if (m_MovingRight || m_MovingLeft) {
                m_TotalFrames = 2;  // Walk cycle: 3 frames (0,1,2)
            } else {
                m_TotalFrames = 0;  // Idle or crouch: single frame
                m_AnimFrame = 0;
            }
        } else {
            m_TotalFrames = 0;  // Jump: single frame
            m_AnimFrame = 0;
        }
    } else {
        m_TotalFrames = 0;
        m_AnimFrame = 0;
    }

    // Update facing direction
    if (m_MovingRight) m_FacingRight = true;
    if (m_MovingLeft) m_FacingRight = false;
}

void PlayerState::SetJumping(bool v) {
    if (v && m_Grounded && !m_Dead) {
        m_LastJumpPoint = {m_PosX, m_PosY};
        m_Grounded = false;
        m_FallHeight = PhysicsEngine::GetJumpHeight(0);
        if (m_PowerState == PowerState::SMALL) {
            Mario::AudioManager::GetInstance().PlaySFX(Mario::SFXName::Jump);
        } else {
            Mario::AudioManager::GetInstance().PlaySFX(Mario::SFXName::BigJump);
        }
    }
}

void PlayerState::ApplyMovement(float speed) {
    if (!m_Controllable || m_Dead) {
        m_VelX = 0.0f;
        return;
    }

    if (m_MovingRight) {
        float s = m_Running ? speed * GameConfig::RUN_MULTIPLIER : speed;
        m_VelX = s;
    } else if (m_MovingLeft) {
        float s = m_Running ? speed * GameConfig::RUN_MULTIPLIER : speed;
        // Prevent going past left boundary
        if (m_PosX >= 5.0f) {
            m_VelX = -s;
        } else {
            m_VelX = 0.0f;
        }
    } else {
        m_VelX = 0.0f;
    }
}

float PlayerState::ApplyGravity() {
    return PhysicsEngine::ApplyGravity(m_FallHeight, m_VelY, m_Grounded);
}

int PlayerState::GetState() const {
    return static_cast<int>(m_Form->GetPowerState());
}

PowerState PlayerState::GetPowerState() const {
    return m_Form->GetPowerState();
}

bool PlayerState::IsBigOrFire() const {
    return m_Form->IsBigOrFire();
}

void PlayerState::SetPowerState(PowerState ps) {
    m_PowerState = ps;
    m_Form = CreatePlayerForm(ps, m_MemoryState);
}

void PlayerState::SetCrouching(bool v) {
    if (m_Crouching == v) return;

    if (IsBigOrFire()) {
        m_PosY += v ? GameConfig::TILE_SIZE : -GameConfig::TILE_SIZE;
    }
    m_Crouching = v;
}

void PlayerState::TakeDamage() {
    if (m_InvTimer > 0 || m_StarTimer > 0) return;  // Already invincible
    if (m_Dead) return;

    auto nextForm = m_Form->TakeDamage();
    if (!nextForm) {
        StartDeathAnimation();
    } else {
        bool wasBig = m_Form->IsBigOrFire();
        m_PrevPowerState = m_PowerState; // Save previous state
        
        m_Form = std::move(nextForm);
        m_PowerState = m_Form->GetPowerState();
        bool isBig = m_Form->IsBigOrFire();

        // Play classic warp/shrink sound
        Mario::AudioManager::GetInstance().PlaySFX(Mario::SFXName::Warp);

        // Invincibility frames
        m_InvTimer = 60;  // ~1.2 seconds of invincibility

        if (wasBig && !isBig) {
            // When shrinking from big to small, adjust Y downwards
            if (!m_Crouching) {
                SetY(GetY() + GameConfig::TILE_SIZE);
            }
            m_TransitionTimer = 30; // Shrink transition
        }
    }
}

void PlayerState::PowerUp(PowerState newState) {
    bool wasBig = IsBigOrFire();
    m_PrevPowerState = m_PowerState; // Save previous state
    
    m_Form = m_Form->PowerUp(newState);
    m_PowerState = m_Form->GetPowerState();
    bool isBig = IsBigOrFire();
    
    if (!wasBig && isBig) {
        SetY(GetY() - GameConfig::TILE_SIZE);
        m_TransitionTimer = 30; // Grow transition from Small to Big/Fire
    } else if (wasBig && newState == PowerState::FIRE) {
        m_TransitionTimer = 30; // Big to Fire transition
    }
}

void PlayerState::StartStar() {
    m_MemoryState = m_Form->GetPowerState();
    if (m_MemoryState == PowerState::SMALL) {
        m_Form = std::make_unique<SmallStarPlayerForm>(m_MemoryState);
    } else {
        m_Form = std::make_unique<BigStarPlayerForm>(m_MemoryState);
    }
    m_PowerState = m_Form->GetPowerState();
    m_StarTimer = 500;  // ~10 seconds
}

// ============================================================================
// ForceApplyPowerState — cheat/debug helper for the ESC power-cycle menu.
// All size-change Y adjustments, star timer setup, and invincibility reset
// live here so no scene handler needs to know the internal rules.
// idx: 0=SMALL, 1=BIG, 2=FIRE, 3=STAR(BIG_STAR)
// ============================================================================
void PlayerState::ForceApplyPowerState(int idx) {
    // Use IsBigOrFire() so the "was 2-tile tall" check is never duplicated.
    bool wasBig = IsBigOrFire();
    bool willBeBig = (idx >= 1);  // BIG / FIRE / STAR are all 2-tile tall

    // Adjust vertical position to keep Mario's feet on the ground.
    if (!wasBig && willBeBig) {
        m_PosY -= static_cast<float>(GameConfig::TILE_SIZE);  // growing up
    } else if (wasBig && !willBeBig) {
        m_PosY += static_cast<float>(GameConfig::TILE_SIZE);  // shrinking down
    }

    // Cancel any pending damage-invincibility; the cheat is instant.
    m_InvTimer = 0;
    m_StarTimer = 0;

    switch (idx) {
        case 0:
            m_PowerState = PowerState::SMALL;
            m_Form = std::make_unique<SmallPlayerForm>();
            break;
        case 1:
            m_PowerState = PowerState::BIG;
            m_Form = std::make_unique<BigPlayerForm>();
            break;
        case 2:
            m_PowerState = PowerState::FIRE;
            m_Form = std::make_unique<FirePlayerForm>();
            break;
        case 3:
            m_PowerState = PowerState::BIG_STAR;
            m_Form = std::make_unique<BigStarPlayerForm>(PowerState::FIRE);
            m_MemoryState = PowerState::FIRE;  // Return to Fire after star ends
            m_StarTimer = 500;
            break;
        default:
            break;
    }
}

int PlayerState::GetWidth() const { return GameConfig::TILE_SIZE; }

int PlayerState::GetHeight() const {
    return m_Form->GetHeight(m_Crouching);
}

bool PlayerState::CanShootFire() const {
    bool cheatActive = false;
    if (ServiceLocator::GetInstance().HasService<GameStateManager>()) {
        auto gs = ServiceLocator::GetInstance().GetService<GameStateManager>();
        if (gs && gs->IsCheatModeActive()) {
            cheatActive = true;
        }
    }
    return cheatActive || m_Form->CanShootFire();
}

AABB PlayerState::GetHitbox() const {
    // Hitbox is slightly thinner (0.6875 ratio, matching C# reference)
    float w = static_cast<float>(GetWidth()) * GameConfig::HITBOX_WIDTH_RATIO;
    float h = static_cast<float>(GetHeight());
    float offsetX = (static_cast<float>(GetWidth()) - w) / 2.0f;
    return AABB::FromPosSize(m_PosX + offsetX, m_PosY, w, h);
}

std::string PlayerState::GetAnimPrefix() const {
    if (m_PoleSliding) return "Pole";

    if (m_SpecialActive) return "Fire";

    // Treat platform-carry as grounded so the Jump sprite never flashes
    // when an ascending platform pushes Mario upward each frame.
    bool effectivelyGrounded = m_Grounded || m_OnMovingPlatform;
    if (effectivelyGrounded) {
        if (m_MovingRight || m_MovingLeft) return "Right";
        if (m_Crouching) return "Crouch";
        return "Idle";
    }
    return "Jump";
}

void PlayerState::SetFireShooting(bool v) {
    // Delegate eligibility to CanShootFire() so this function is never
    // coupled to the list of power states that grant fire ability.
    if (v && CanShootFire() && !m_SpecialActive) {
        m_SpecialActive = true;
        m_SpecialCounter = 0;
        Mario::AudioManager::GetInstance().PlaySFX(Mario::SFXName::FireBall);
    }
}

void PlayerState::StartDeathAnimation() {
    if (m_DeathAnimation && m_DeathAnimation->IsActive()) return;

    m_Dead = true;
    m_Controllable = false;
    m_VelX = 0.0f;
    m_VelY = 0.0;

    if (!m_DeathAnimation) {
        m_DeathAnimation = std::make_unique<ClassicPlayerDeathAnimation>();
    }
    m_DeathAnimation->Start();
}

void PlayerState::UpdateDeathAnimation() {
    if (!m_DeathAnimation || !m_DeathAnimation->IsActive()) return;

    m_VelX = 0.0f;
    m_VelY = 0.0;
    m_DeathAnimation->Tick(GameConfig::GRAVITY, GameConfig::TICK_INTERVAL,
                           GameConfig::JUMP_VELOCITY, m_PosY);
}

}  // namespace Mario
