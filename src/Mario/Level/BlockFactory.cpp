/**
 * @file BlockFactory.cpp
 * @brief Implementation of BlockFactory class.
 * @inheritance None
 */
#include "Mario/Level/BlockFactory.hpp"

#include "Mario/Level/MovingPlatform.hpp"

namespace Mario {

std::shared_ptr<Block> BlockFactory::CreateBlock(int blockID, int gridX, int gridY,
                                                 const BlockDef& def,
                                                 const std::string& levelName) {
    if (blockID == 960) {
        return std::make_shared<MovingPlatform>(
            blockID, gridX, gridY, def, MovingPlatform::Direction::VERTICAL,
            levelName);
    }
    if (blockID == 961) {
        return std::make_shared<MovingPlatform>(
            blockID, gridX, gridY, def, MovingPlatform::Direction::HORIZONTAL,
            levelName);
    }

    if (blockID == GameConfig::PIPE_DOWN_LEFT || blockID == GameConfig::PIPE_DOWN_RIGHT ||
        blockID == GameConfig::PIPE_RIGHT_TOP || blockID == GameConfig::PIPE_RIGHT_BOT) {
        return std::make_shared<WarpPipeBlock>(blockID, gridX, gridY, def, levelName);
    }

    if (def.isGoal) {
        return std::make_shared<GoalBlock>(blockID, gridX, gridY, def, levelName);
    }
    if (def.name == "Bridge" || def.name == "BridgeBlock") {
        return std::make_shared<BridgeBlock>(blockID, gridX, gridY, def, levelName);
    }
    if (def.background) {
        return std::make_shared<BackgroundBlock>(blockID, gridX, gridY, def, levelName);
    }
    if (def.name == "InvisQuestionBlock") {
        return std::make_shared<InvisibleBlock>(blockID, gridX, gridY, def, levelName);
    }
    if (def.isContainer) {
        return std::make_shared<QuestionBlock>(blockID, gridX, gridY, def, levelName);
    }
    if (def.breakable) {
        return std::make_shared<BrickBlock>(blockID, gridX, gridY, def, levelName);
    }

    // Default fallback to StoneBlock
    return std::make_shared<StoneBlock>(blockID, gridX, gridY, def, levelName);
}

}  // namespace Mario
