/**
 * @file LevelCompleteController.cpp
 * @brief Implementation of level completion sequences.
 *        Flagpole: slide down pole -> walk right -> enter castle -> next level.
 *        Pipe warp: descend/move right -> load sub-level or exit warp.
 *        Ported from C# Form1.cs (lines 941-968, 1174-1265).
 * @inheritance None (Controller in MVC)
 */
#include "Mario/LevelCompleteController.hpp"

#include "Util/Logger.hpp"

namespace Mario {

void LevelCompleteController::Reset() {
    m_Phase = EndingPhase::NONE;
    m_was_pipe_warp = false;
    m_flag_entity = nullptr;
    m_bowser = nullptr;
    m_princess = nullptr;
    m_end_timer = -1;
    m_level_timer = -1;
    m_tick_count = 0;
}

// ============================================================================
// Flagpole Sequence Start
// C# reference: Form1.cs lines 1231-1265
// ============================================================================
void LevelCompleteController::StartFlagpole(Player& player,
                                            std::shared_ptr<Entity> flagEntity,
                                            const Block* goalBlock) {
    m_Phase = EndingPhase::POLE_SLIDE;
    m_was_pipe_warp = false;
    m_flag_entity = flagEntity;
    m_tick_count = 0;

    if (goalBlock) {
        m_goal_block_x = goalBlock->GetWorldX();
        m_goal_block_y = goalBlock->GetWorldY();
    }

    // Disable player control (C# line 1250: Mario.SetControllable(false))
    player.GetState().SetControllable(false);

    // Set pole sliding state
    player.GetState().SetPoleSliding(true);

    // Snap Mario exactly to grip the pole (shift left so visual connects)
    // We adjust position relative to the block so Mario's right hand touches it
    float poleX = m_goal_block_x - GameConfig::TILE_SIZE * 0.4f;
    player.GetState().SetX(poleX);

    // We do NOT override Mario's Y position (C# lines 1249:
    // Mario.ChangePosition(X + offset, Y)) This allows Mario to slide down from
    // where he contacted the flagpole.

    player.GetState().SetVelX(0.0f);
    player.GetState().SetVelY(0.0f);

    // Stop star if active (C# line 1237-1239)
    if (player.GetState().GetPowerState() == PowerState::SMALL_STAR ||
        player.GetState().GetPowerState() == PowerState::BIG_STAR) {
        player.GetState().SetPowerState(player.GetState().GetPowerState() ==
                                                PowerState::BIG_STAR
                                            ? PowerState::BIG
                                            : PowerState::SMALL);
    }

    LOG_INFO("Flagpole sequence started at ({}, {})", m_goal_block_x,
             m_goal_block_y);
}

// ============================================================================
// 8-4 Axe Sequence Start
// ============================================================================
void LevelCompleteController::StartAxeSequence(
    Player& player, std::shared_ptr<Entity> bowser,
    std::shared_ptr<Entity> princess) {
    m_Phase = EndingPhase::AXE_SEQUENCE_START;
    m_was_pipe_warp = false;
    m_tick_count = 0;
    m_bowser = bowser;
    m_princess = princess;

    player.GetState().SetControllable(false);
    player.GetState().SetVelX(0.0f);
    player.GetState().SetVelY(0.0);
    player.GetState().SetMovingRight(false);
    player.GetState().SetMovingLeft(false);
    player.GetState().SetPoleSliding(false);
    player.SetVisible(true);

    LOG_INFO("8-4 Axe sequence started.");
}

// ============================================================================
// Pipe Warp Sequence Start
// ============================================================================
void LevelCompleteController::StartPipeWarp(Player& player,
                                            const std::string& direction,
                                            float pipeWorldX,
                                            float pipeWorldY) {
    m_was_pipe_warp = true;
    m_pipe_direction = direction;
    m_pipe_x = pipeWorldX;
    m_pipe_y = pipeWorldY;
    m_tick_count = 0;

    // Disable control (C# line 945: Mario.SetControllable(false))
    player.GetState().SetControllable(false);
    player.GetState().SetVelX(0.0f);
    player.GetState().SetVelY(0.0);

    // Drop ZIndex so Mario renders BEHIND the pipe tiles.
    // Since solid blocks are at GameConfig::Z_BLOCK, we drop player Z-index
    // below that, but above background (-10.0f) to remain in front of
    // background. The player object is recreated when the next level loads, so
    // no explicit ZIndex restore is needed.
    player.SetZIndex(GameConfig::Z_BLOCK - 1.0f);

    if (direction == "Down") {
        m_Phase = EndingPhase::PIPE_DESCEND;
        // Mario descends 2 tiles below pipe
        m_pipe_target_y = pipeWorldY + GameConfig::TILE_SIZE * 2;
        LOG_INFO("Pipe warp DOWN started at ({}, {})", pipeWorldX, pipeWorldY);
    } else {
        m_Phase = EndingPhase::PIPE_RIGHT;
        // Mario walks 2 tiles into the pipe
        m_pipe_target_x = pipeWorldX + GameConfig::TILE_SIZE * 2;
        player.GetState().SetGrounded(true);
        LOG_INFO("Pipe warp RIGHT started at ({}, {})", pipeWorldX, pipeWorldY);
    }
}

// ============================================================================
// Per-frame Update
// ============================================================================
bool LevelCompleteController::Update(Player& player, Level& level,
                                     float cameraOffset) {
    if (m_Phase == EndingPhase::NONE || m_Phase == EndingPhase::COMPLETED) {
        return false;
    }

    m_tick_count++;

    switch (m_Phase) {
        case EndingPhase::POLE_SLIDE:
            UpdatePoleSlide(player);
            break;
        case EndingPhase::POLE_WALK:
            UpdatePoleWalk(player, level);
            break;
        case EndingPhase::ENTER_CASTLE:
            UpdateEnterCastle(player);
            break;
        case EndingPhase::WAIT_TRANSITION:
            if (m_tick_count > m_level_timer) {
                m_Phase = EndingPhase::COMPLETED;
            }
            break;
        case EndingPhase::PIPE_DESCEND:
            UpdatePipeDescend(player);
            break;
        case EndingPhase::PIPE_RIGHT:
            UpdatePipeRight(player);
            break;
        case EndingPhase::AXE_SEQUENCE_START:
        case EndingPhase::BRIDGE_COLLAPSE:
        case EndingPhase::BOWSER_FALL:
        case EndingPhase::WALK_TO_PRINCESS:
        case EndingPhase::PRINCESS_DIALOG:
            UpdateAxeSequence(player, level);
            break;
        default:
            break;
    }

    // Update view
    player.UpdateView(cameraOffset);

    return IsActive();
}

// ============================================================================
// Flagpole: Slide Down
// C# reference: Form1.cs lines 1252-1255
// Mario slides down pole at FLAG_SPEED until he reaches ground
// ============================================================================
void LevelCompleteController::UpdatePoleSlide(Player& player) {
    PlayerState& ps = player.GetState();

    // Slide Mario down the pole if not grounded
    if (!ps.IsGrounded()) {
        float marioY = ps.GetY() + GameConfig::FLAGPOLE_SLIDE_SPEED;
        const float groundSurfaceY = 13.0f * GameConfig::TILE_SIZE;
        if (marioY + ps.GetHeight() >= groundSurfaceY) {
            marioY = groundSurfaceY - ps.GetHeight();
            ps.SetGrounded(true);
            ps.SetVelY(0.0f);
        }
        ps.SetY(marioY);
    }

    // Slide flag entity down with Mario/until bottom
    // C#: Flag.Y <= Mario.Y
    if (m_flag_entity && m_flag_entity->GetState().IsActive()) {
        EntityState& fs = m_flag_entity->GetState();
        if (fs.GetY() <= ps.GetY()) {
            float flagY = fs.GetY() + GameConfig::FLAGPOLE_SLIDE_SPEED;
            fs.SetY(flagY);
        }
    }

    // Check transition to POLE_WALK (which acts as the delay + walk phase)
    // C#: Flag.Y > Mario.Y && Mario.GetDirection() == "Right" && isGrounded
    if (ps.IsGrounded()) {
        bool flagPassedMario = true;
        if (m_flag_entity && m_flag_entity->GetState().IsActive()) {
            flagPassedMario = (m_flag_entity->GetState().GetY() > ps.GetY());
        }
        if (flagPassedMario && ps.IsFacingRight()) {
            // Shift Mario right to the other side of the pole
            ps.SetX(ps.GetX() + GameConfig::TILE_SIZE * 0.6f);
            ps.SetFacingRight(false);  // Flip facing left
            m_Phase = EndingPhase::POLE_WALK;
            m_tick_count = 0;  // Reset tick count for the 20-tick delay
            LOG_INFO("Flagpole slide complete - Transitioning to POLE_WALK");
        }
    }
}

// ============================================================================
// Flagpole: Walk to Castle
// C# reference: Form1.cs lines 1215-1250
// After sliding down pole, Mario waits then walks toward castle
// ============================================================================
void LevelCompleteController::UpdatePoleWalk(Player& player, Level& level) {
    PlayerState& ps = player.GetState();

    // Keep Mario grounded and his bottom exactly at ground level throughout
    const float groundSurfaceY = 13.0f * GameConfig::TILE_SIZE;
    ps.SetY(groundSurfaceY - ps.GetHeight());
    ps.SetGrounded(true);
    ps.SetVelY(0);

    // C# reference (Form1.cs lines 1244-1247):
    // Wait 20 ticks (0.4 seconds) before starting to walk
    if (m_tick_count < GameConfig::ENDING_WALK_DELAY) {
        // Just wait, don't move. Facing left, holding pole.
        ps.SetVelX(0);
        ps.SetMovingRight(false);
        return;
    }

    // After 20 ticks, Mario turns right, let's exit pole state
    if (ps.IsPoleSliding()) {
        ps.SetPoleSliding(false);
        ps.SetFacingRight(true);
    }

    // Walk right toward the castle at normal Mario speed
    float castleWalkSpeed = GameConfig::SCALED_SPEED;
    ps.SetX(ps.GetX() + castleWalkSpeed);
    ps.SetMovingRight(true);
    ps.SetFacingRight(true);

    // Look for the Castle5 (door) block to detect when to enter castle
    for (const auto& block : level.GetAllBlocks()) {
        if (block && (block->GetName() == "Castle5" ||
                      block->GetName() == "CastleDoor")) {
            float castleX = block->GetWorldX();
            // Stop when reached the castle door
            if (ps.GetX() >= castleX) {
                // Transition to entering castle
                m_Phase = EndingPhase::ENTER_CASTLE;
                m_tick_count = 0;
                ps.SetMovingRight(false);
                ps.SetVelX(0);
                LOG_INFO("Flagpole walk complete - reached castle at X={}",
                         castleX);
                return;
            }
        }
    }

    // Fallback: if no castle block found, walk a fixed distance
    if (ps.GetX() > m_goal_block_x + GameConfig::TILE_SIZE * 8) {
        m_Phase = EndingPhase::ENTER_CASTLE;
        m_tick_count = 0;
        ps.SetMovingRight(false);
        ps.SetVelX(0);
        LOG_WARN("Castle block not found - using fallback distance");
    }
}

// ============================================================================
// Enter Castle: Make Mario invisible and wait
// C# reference: Form1.cs lines 1221-1225
// ============================================================================
void LevelCompleteController::UpdateEnterCastle(Player& player) {
    // Make Mario invisible (C# line 1221: setRecBox to 0,0)
    player.SetVisible(false);

    if (m_level_timer < 0) {
        m_level_timer = m_tick_count + GameConfig::LEVEL_TRANSITION_DELAY;
    }

    m_Phase = EndingPhase::WAIT_TRANSITION;
    LOG_DEBUG("Mario entered castle, waiting {} ticks",
              GameConfig::LEVEL_TRANSITION_DELAY);
}

// ============================================================================
// Pipe: Descend (Down direction)
// C# reference: Form1.cs lines 946-955
// ============================================================================
void LevelCompleteController::UpdatePipeDescend(Player& player) {
    PlayerState& ps = player.GetState();

    // Move Mario downward (C# line 948: Mario.SetFltYVel(-5))
    float newY = ps.GetY() + GameConfig::PIPE_ANIM_SPEED;
    ps.SetY(newY);

    // Once Mario has sunk one full tile into the pipe, make him invisible.
    // The ZIndex=-1 (set in StartPipeWarp) already puts him behind the pipe
    // tiles visually; SetVisible(false) hides any portion still peeking above.
    if (ps.GetY() > m_pipe_y + GameConfig::TILE_SIZE) {
        player.SetVisible(false);
    }

    // Check if descended far enough
    if (ps.GetY() > m_pipe_target_y) {
        m_Phase = EndingPhase::COMPLETED;
        LOG_DEBUG("Pipe descend complete");
    }
}

// ============================================================================
// Pipe: Walk Right
// C# reference: Form1.cs lines 956-965
// ============================================================================
void LevelCompleteController::UpdatePipeRight(Player& player) {
    PlayerState& ps = player.GetState();

    // Move Mario right
    float newX = ps.GetX() + GameConfig::PIPE_ANIM_SPEED;
    ps.SetX(newX);
    ps.SetMovingRight(true);
    ps.SetFacingRight(true);

    // Once Mario has walked one tile into the pipe, make him invisible.
    // Combined with ZIndex=-1, this prevents the ghost/pass-through look.
    if (ps.GetX() > m_pipe_x + GameConfig::TILE_SIZE) {
        player.SetVisible(false);
    }

    // Check if moved far enough
    if (ps.GetX() > m_pipe_target_x) {
        m_Phase = EndingPhase::COMPLETED;
        LOG_DEBUG("Pipe right-walk complete");
    }
}

// ============================================================================
// 8-4: Axe Sequence Update
// ============================================================================
void LevelCompleteController::UpdateAxeSequence(Player& player, Level& level) {
    switch (m_Phase) {
        case EndingPhase::AXE_SEQUENCE_START:
            // Brief pause after touching axe
            if (m_tick_count > 30) {  // ~0.5s pause
                m_Phase = EndingPhase::BRIDGE_COLLAPSE;
                m_tick_count = 0;
                LOG_INFO("8-4: Collapsing bridge.");

                // Make bridge blocks fall
                for (const auto& block : level.GetAllBlocks()) {
                    if (block && (block->GetName() == "Bridge" ||
                                  block->GetName() == "BridgeBlock")) {
                        block->SetGravity(true);
                        block->SetCollidable(false);
                    }
                }
            }
            break;

        case EndingPhase::BRIDGE_COLLAPSE:
            // Wait for bridge to fall a bit, then make Bowser fall
            if (m_tick_count > 60) {  // ~1s
                m_Phase = EndingPhase::BOWSER_FALL;
                m_tick_count = 0;
                if (m_bowser && m_bowser->GetState().IsActive()) {
                    LOG_INFO("8-4: Bowser falls.");
                    auto& bowserState = m_bowser->GetState();
                    m_bowser->SetBehavior(nullptr);  // Disable AI
                    bowserState.SetCollidable(false);
                    bowserState.SetGravity(true);
                    bowserState.SetVelY(-5.0f);  // Give a little push
                }
            }
            break;

        case EndingPhase::BOWSER_FALL:
            // After Bowser is off-screen, start walking to princess
            if (m_tick_count > 180) {  // ~3s
                m_Phase = EndingPhase::WALK_TO_PRINCESS;
                m_tick_count = 0;
                LOG_INFO("8-4: Walking to Princess.");
            }
            break;

        case EndingPhase::WALK_TO_PRINCESS: {
            PlayerState& ps = player.GetState();
            // Let gravity act so Mario drops down to the lower floor naturally
            ps.SetMovingRight(true);
            ps.SetFacingRight(true);
            ps.SetX(ps.GetX() +
                    GameConfig::SCALED_SPEED * 0.5f);  // Walk slower

            if (m_princess && m_princess->GetState().IsActive()) {
                float princessX = m_princess->GetState().GetX();
                if (ps.GetX() >= princessX - ps.GetWidth()) {
                    ps.SetMovingRight(false);
                    ps.SetVelX(0);
                    m_Phase = EndingPhase::PRINCESS_DIALOG;
                    m_tick_count = 0;
                    LOG_INFO("8-4: Reached Princess.");
                }
            } else {                       // Fallback if princess not found
                if (m_tick_count > 300) {  // Walk for ~5s
                    m_Phase = EndingPhase::PRINCESS_DIALOG;
                }
            }
            break;
        }

        case EndingPhase::PRINCESS_DIALOG:
            // Wait for a few seconds then end the game
            if (m_tick_count > 300) {  // ~5s
                m_Phase = EndingPhase::COMPLETED;
                LOG_INFO("8-4: Game complete!");
            }
            break;

        default:
            break;
    }
}

}  // namespace Mario
