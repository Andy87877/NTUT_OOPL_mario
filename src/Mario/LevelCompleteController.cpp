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

    // Snap Mario to the pole position (C# line 1249: offset by scaleSize/2.5)
    float poleX = m_GoalBlockX + GameConfig::TILE_SIZE * 0.4f;
    player.GetState().SetX(poleX);
    player.GetState().SetVelX(0.0f);
    player.GetState().SetVelY(0.0);

    // Stop star if active (C# line 1237-1239)
    if (player.GetState().GetPowerState() == PowerState::SMALL_STAR ||
        player.GetState().GetPowerState() == PowerState::BIG_STAR) {
        player.GetState().SetPowerState(
            player.GetState().GetPowerState() == PowerState::BIG_STAR
                ? PowerState::BIG : PowerState::SMALL);
    }

    LOG_INFO("Flagpole sequence started at ({}, {})", m_GoalBlockX, m_GoalBlockY);
}

// ============================================================================
// Pipe Warp Sequence Start
// C# reference: Form1.cs lines 941-968
// ============================================================================
void LevelCompleteController::StartPipeWarp(Player& player,
                                             const std::string& direction,
                                             float pipeWorldX, float pipeWorldY) {
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

    // Slide Mario down
    if (!ps.IsGrounded()) {
        float newY = ps.GetY() + GameConfig::FLAGPOLE_SLIDE_SPEED;
        ps.SetY(newY);
    }

    // Slide flag entity down with Mario
    if (m_FlagEntity && m_FlagEntity->GetState().IsActive()) {
        EntityState& fs = m_FlagEntity->GetState();
        if (fs.GetY() <= ps.GetY()) {
            fs.SetY(fs.GetY() + GameConfig::FLAGPOLE_SLIDE_SPEED);
        }
    }

    // Check if Mario reached the ground
    // (ground is at row 13 in a 16-row level = 13*32 = 416)
    if (ps.IsGrounded() || ps.GetY() >= (GameConfig::LEVEL_ROWS - 3) * GameConfig::TILE_SIZE) {
        ps.SetGrounded(true);
        ps.SetPoleSliding(false);

        // C# lines 1258-1263: Flip direction and start walking
        // After landing, Mario turns around briefly then walks right
        m_Phase = EndingPhase::POLE_WALK;
        m_TickCount = 0;
        LOG_DEBUG("Flagpole slide complete, starting walk phase");
    }
}

// ============================================================================
// Flagpole: Walk to Castle
// C# reference: Form1.cs lines 1206-1228
// Mario walks right until reaching Castle5 block
// ============================================================================
void LevelCompleteController::UpdatePoleWalk(Player& player, Level& level) {
    PlayerState& ps = player.GetState();

    // Brief delay before walking (C# line 1262: endTime = timer + 20)
    if (m_TickCount < GameConfig::ENDING_WALK_DELAY) {
        return;
    }

    // Walk right toward the castle
    float walkSpeed = GameConfig::SCALED_SPEED;
    ps.SetX(ps.GetX() + walkSpeed);
    ps.SetMovingRight(true);

    // Look for Castle5 block to determine stop position
    // In the C# ref, the castle door is named "Castle5"
    bool foundCastle = false;
    for (const auto& block : level.GetAllBlocks()) {
        if (block->GetName() == "Castle5" || block->GetName() == "CastleDoor") {
            float castleX = block->GetWorldX();
            if (ps.GetX() >= castleX) {
                m_Phase = EndingPhase::ENTER_CASTLE;
                m_TickCount = 0;
                foundCastle = true;
                LOG_DEBUG("Reached castle at X={}", castleX);
                break;
            }
        }
    }

    // Fallback: if no castle block, walk a fixed distance past the goal
    if (!foundCastle && ps.GetX() > m_GoalBlockX + GameConfig::TILE_SIZE * 8) {
        m_Phase = EndingPhase::ENTER_CASTLE;
        m_TickCount = 0;
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
    LOG_DEBUG("Mario entered castle, waiting {} ticks", GameConfig::LEVEL_TRANSITION_DELAY);
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

} // namespace Mario
