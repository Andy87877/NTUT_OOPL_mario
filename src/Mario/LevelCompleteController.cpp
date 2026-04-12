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
    m_WasPipeWarp = false;
    m_FlagEntity = nullptr;
    m_EndTimer = -1;
    m_LevelTimer = -1;
    m_TickCount = 0;
}

// ============================================================================
// Flagpole Sequence Start
// C# reference: Form1.cs lines 1231-1265
// ============================================================================
void LevelCompleteController::StartFlagpole(Player& player,
                                            std::shared_ptr<Entity> flagEntity,
                                            const Block* goalBlock) {
    m_Phase = EndingPhase::POLE_SLIDE;
    m_WasPipeWarp = false;
    m_FlagEntity = flagEntity;
    m_TickCount = 0;

    if (goalBlock) {
        m_GoalBlockX = goalBlock->GetWorldX();
        m_GoalBlockY = goalBlock->GetWorldY();
    }

    // Disable player control (C# line 1250: Mario.SetControllable(false))
    player.GetState().SetControllable(false);

    // Set pole sliding state
    player.GetState().SetPoleSliding(true);

    // Snap Mario exactly to grip the pole (shift left so visual connects)
    // We adjust position relative to the block so Mario's right hand touches it
    float poleX = m_GoalBlockX - GameConfig::TILE_SIZE * 0.4f;
    player.GetState().SetX(poleX);

    // Set Mario's Y to flagpole's Y position (start of descent)
    if (flagEntity) {
        float poleY = flagEntity->GetState().GetY();
        player.GetState().SetY(poleY);
        LOG_DEBUG("Starting pole slide from Y={}", poleY);
    }

    player.GetState().SetVelX(0.0f);
    player.GetState().SetVelY(0.0);

    // Mario must NOT be grounded during pole slide phase
    // This allows UpdatePoleSlide to trigger descent (checks !ps.IsGrounded())
    player.GetState().SetGrounded(false);

    // Stop star if active (C# line 1237-1239)
    if (player.GetState().GetPowerState() == PowerState::SMALL_STAR ||
        player.GetState().GetPowerState() == PowerState::BIG_STAR) {
        player.GetState().SetPowerState(player.GetState().GetPowerState() ==
                                                PowerState::BIG_STAR
                                            ? PowerState::BIG
                                            : PowerState::SMALL);
    }

    LOG_INFO("Flagpole sequence started at ({}, {})", m_GoalBlockX,
             m_GoalBlockY);
}

// ============================================================================
// Pipe Warp Sequence Start
// ============================================================================
void LevelCompleteController::StartPipeWarp(Player& player,
                                            const std::string& direction,
                                            float pipeWorldX,
                                            float pipeWorldY) {
    m_WasPipeWarp = true;
    m_PipeDirection = direction;
    m_PipeX = pipeWorldX;
    m_PipeY = pipeWorldY;
    m_TickCount = 0;

    // Disable control (C# line 945: Mario.SetControllable(false))
    player.GetState().SetControllable(false);
    player.GetState().SetVelX(0.0f);
    player.GetState().SetVelY(0.0);

    if (direction == "Down") {
        m_Phase = EndingPhase::PIPE_DESCEND;
        // Mario descends 2 tiles below pipe
        m_PipeTargetY = pipeWorldY + GameConfig::TILE_SIZE * 2;
        LOG_INFO("Pipe warp DOWN started at ({}, {})", pipeWorldX, pipeWorldY);
    } else {
        m_Phase = EndingPhase::PIPE_RIGHT;
        // Mario walks 2 tiles into the pipe
        m_PipeTargetX = pipeWorldX + GameConfig::TILE_SIZE * 2;
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

    m_TickCount++;

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
            if (m_TickCount > m_LevelTimer) {
                m_Phase = EndingPhase::COMPLETED;
            }
            break;
        case EndingPhase::PIPE_DESCEND:
            UpdatePipeDescend(player);
            break;
        case EndingPhase::PIPE_RIGHT:
            UpdatePipeRight(player);
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

    // C# Form1.cs line 1252-1255 flagpole ending sequence:
    // Flag slides down, Mario slides down
    // Both move at: FLAG_SPEED = 2 (pixels per tick)

    // Slide Mario down the pole
    // Mario must NOT be grounded during pole slide (controlled descent)
    float marioY = ps.GetY() + GameConfig::FLAGPOLE_SLIDE_SPEED;
    ps.SetY(marioY);
    ps.SetGrounded(false);  // Stay ungrounded while sliding

    // Slide flag entity down with Mario
    // Flag continues sliding as long as its Y is above Mario
    if (m_FlagEntity && m_FlagEntity->GetState().IsActive()) {
        EntityState& fs = m_FlagEntity->GetState();
        // Flag slides parallel to Mario
        float flagY = fs.GetY() + GameConfig::FLAGPOLE_SLIDE_SPEED;
        fs.SetY(flagY);
    }

    // Check if Mario reached the ground (ground blocks in row 13)
    // Ground level Y = 13 * 45 = 585 pixels. This is where Mario's BOTTOM
    // should rest.
    const float groundSurfaceY = 13.0f * GameConfig::TILE_SIZE;

    // Mario's bottom is Y + height
    if (ps.GetY() + ps.GetHeight() >= groundSurfaceY) {
        // Mario hit the ground
        ps.SetY(groundSurfaceY -
                ps.GetHeight());  // Snap his bottom to the ground surface
        ps.SetGrounded(true);     // Now grounded
        ps.SetVelY(0);
        ps.SetPoleSliding(false);

        // Transition to walking phase (C# lines 1256-1258)
        m_Phase = EndingPhase::POLE_WALK;
        m_TickCount = 0;
        LOG_INFO("Flagpole slide complete - Mario landed at bottom Y={}",
                 groundSurfaceY);
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
    // walk phase
    const float groundSurfaceY = 13.0f * GameConfig::TILE_SIZE;
    ps.SetY(groundSurfaceY - ps.GetHeight());
    ps.SetGrounded(true);
    ps.SetVelY(0);

    // C# reference (Form1.cs lines 1244-1247):
    // Wait 20 ticks (0.4 seconds) before starting to walk
    if (m_TickCount < GameConfig::ENDING_WALK_DELAY) {
        // Just wait, don't move
        ps.SetVelX(0);
        ps.SetMovingRight(false);
        return;
    }

    // Walk right toward the castle at normal Mario speed
    // C# Form1.cs 1218: moveRight(Mario)
    // This applies SCALED_SPEED to Mario's X position
    float castleWalkSpeed = GameConfig::SCALED_SPEED;
    ps.SetX(ps.GetX() + castleWalkSpeed);
    ps.SetMovingRight(true);
    ps.SetFacingRight(true);

    // Look for the Castle5 (door) block to detect when to enter castle
    // C# Form1.cs lines 1219-1227: searches for block named "Castle5"
    for (const auto& block : level.GetAllBlocks()) {
        if (block && (block->GetName() == "Castle5" ||
                      block->GetName() == "CastleDoor")) {
            float castleX = block->GetWorldX();
            // Stop when reached the castle door
            if (ps.GetX() >= castleX) {
                // Transition to entering castle
                m_Phase = EndingPhase::ENTER_CASTLE;
                m_TickCount = 0;
                ps.SetMovingRight(false);
                ps.SetVelX(0);
                LOG_INFO("Flagpole walk complete - reached castle at X={}",
                         castleX);
                return;
            }
        }
    }

    // Fallback: if no castle block found, walk a fixed distance
    if (ps.GetX() > m_GoalBlockX + GameConfig::TILE_SIZE * 8) {
        m_Phase = EndingPhase::ENTER_CASTLE;
        m_TickCount = 0;
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

    if (m_LevelTimer < 0) {
        m_LevelTimer = m_TickCount + GameConfig::LEVEL_TRANSITION_DELAY;
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

    // Check if descended far enough
    if (ps.GetY() > m_PipeTargetY) {
        m_Phase = EndingPhase::COMPLETED;
        LOG_DEBUG("Pipe descend complete");
    }
}

// ============================================================================
// Pipe: Walk Right
// C# reference: Form1.cs lines 957-968
// ============================================================================
void LevelCompleteController::UpdatePipeRight(Player& player) {
    PlayerState& ps = player.GetState();

    // Move Mario right (C# line 959: moveRight(Mario))
    float newX = ps.GetX() + GameConfig::SCALED_SPEED;
    ps.SetX(newX);
    ps.SetGrounded(true);

    // Check if walked far enough
    if (ps.GetX() > m_PipeTargetX) {
        m_Phase = EndingPhase::COMPLETED;
        LOG_DEBUG("Pipe walk right complete");
    }
}

}  // namespace Mario
