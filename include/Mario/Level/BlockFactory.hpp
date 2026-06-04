/**
 * @file BlockFactory.hpp
 * @brief Factory class for creating concrete Block instances based on definitions.
 * @inheritance None (Factory Pattern)
 */
#ifndef MARIO_BLOCK_FACTORY_HPP
#define MARIO_BLOCK_FACTORY_HPP

#include <memory>
#include <string>
#include <vector>

#include "Mario/Level/Block.hpp"

namespace Mario {

/**
 * Creates concrete block subclass instances based on Block ID and definitions.
 * Decouples Level logic from concrete subclasses of Block.
 */
class BlockFactory {
   public:
    BlockFactory() = delete;
    ~BlockFactory() = delete;

    /**
     * Create a concrete Block subclass instance.
     * @param blockID The block ID from grid
     * @param gridX Column index
     * @param gridY Row index
     * @param def The BlockDef configuration data
     * @param levelName Current level name
     * @return std::shared_ptr<Block> Constructed block subclass
     */
    static std::shared_ptr<Block> CreateBlock(int blockID, int gridX, int gridY,
                                              const BlockDef& def,
                                              const std::string& levelName);
};

}  // namespace Mario

#endif  // MARIO_BLOCK_FACTORY_HPP
