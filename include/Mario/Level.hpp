/**
 * @file Level.hpp
 * @brief Level class that parses CSV level data and manages the block grid.
 *        Responsible for loading, storing, and providing access to all blocks.
 * @inheritance None (manager class)
 */
#ifndef MARIO_LEVEL_HPP
#define MARIO_LEVEL_HPP

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "Mario/Block.hpp"
#include "Mario/EntityDef.hpp"
#include "Mario/MovingPlatform.hpp"

namespace Mario {

/**
 * Manages a single level's tile grid and entity spawn points.
 *
 * Responsibilities:
 *   - Parse level CSV into a 2D grid of block IDs
 *   - Parse IDList.csv and EntityList.csv lookup tables
 *   - Create Block instances for each non-empty cell
 *   - Provide block access by grid position
 *   - Track entity spawn points for EntityFactory
 *   - Handle level dimensions and boundaries
 */
class Level {
   public:
    /**
     * Spawn point data from level CSV (GoombaSpawn, KoopaSpawn, etc.)
     */
    struct SpawnPoint {
        int entityID;
        std::string entityName;
        int gridX;
        int gridY;
        float worldX;
        float worldY;
        bool spawned = false;  // Only spawn once
    };

    Level();
    ~Level() = default;

    /**
     * Load and parse a level from CSV file.
     * @param levelName Level name without extension (e.g., "1-1")
     * @return true if successfully loaded
     */
    bool Load(const std::string& levelName);

    /**
     * Update all blocks (animations, visibility culling).
     * @param cameraOffset Camera scroll offset
     */
    void UpdateBlocks(float cameraOffset);

    /**
     * Get block at grid position.
     * @return nullptr if out of bounds or empty
     */
    Block* GetBlockAt(int gridX, int gridY);
    const Block* GetBlockAt(int gridX, int gridY) const;

    /**
     * Get block at world position (pixel coordinates).
     */
    Block* GetBlockAtWorld(float worldX, float worldY);
    const Block* GetBlockAtWorld(float worldX, float worldY) const;

    /**
     * Get all blocks in range (for rendering / collision detection).
     */
    std::vector<Block*> GetBlocksInRange(float leftX, float rightX) const;

    /**
     * Get all spawn points in the level.
     */
    const std::vector<SpawnPoint>& GetSpawnPoints() const {
        return m_SpawnPoints;
    }
    std::vector<SpawnPoint>& GetSpawnPoints() { return m_SpawnPoints; }

    /**
     * Get all blocks as a flat list (for Renderer).
     */
    const std::vector<std::shared_ptr<Block>>& GetAllBlocks() const {
        return m_Blocks;
    }

    /**
     * Get all moving platform instances (subset of m_Blocks).
     * Used by CollisionManager and PlayingSceneHandler for carry logic.
     */
    const std::vector<MovingPlatform*>& GetMovingPlatforms() const {
        return m_MovingPlatforms;
    }

    /**
     * Look up a block definition by ID.
     */
    const BlockDef& GetBlockDef(int id) const;

    /**
     * Look up an entity definition by ID.
     */
    const EntityDef& GetEntityDef(int id) const;
    const EntityDef& GetEntityDefByName(const std::string& name) const;

    // -- Level Properties --
    int GetWidth() const { return m_Width; }
    int GetHeight() const { return m_Height; }
    float GetWidthPixels() const {
        return static_cast<float>(m_Width * GameConfig::TILE_SIZE);
    }
    float GetHeightPixels() const {
        return static_cast<float>(m_Height * GameConfig::TILE_SIZE);
    }

    /**
     * Get the player spawn position (grid coords from MARIO_SPAWN_ID).
     */
    float GetPlayerSpawnX() const { return m_PlayerSpawnX; }
    float GetPlayerSpawnY() const { return m_PlayerSpawnY; }

    /**
     * Check if the level has an underground sub-level.
     */
    bool HasSubLevel() const { return !m_SubLevelName.empty(); }
    const std::string& GetSubLevelName() const { return m_SubLevelName; }
    float GetPipeExitX() const { return m_PipeExitX; }

    /**
     * Get the current level name.
     */
    const std::string& GetLevelName() const { return m_LevelName; }

   private:
    // -- Parsing --
    bool ParseLevelCSV(const std::string& path);
    void LoadLookupTables();
    void CreateBlocksFromGrid();
    void IdentifySpawnPoints();
    std::vector<std::string> SplitCSVLine(const std::string& line) const;

    // -- Data --
    std::string m_LevelName;
    std::string m_SubLevelName;

    int m_Width = 0;
    int m_Height = 0;

    // 2D grid of block IDs [row][col]
    std::vector<std::vector<int>> m_Grid;

    // All block instances
    std::vector<std::shared_ptr<Block>> m_Blocks;

    // Non-owning pointers to moving platforms (owned by m_Blocks)
    std::vector<MovingPlatform*> m_MovingPlatforms;

    // Quick lookup by grid position: key = (gridY * MAX_WIDTH + gridX)
    std::unordered_map<int, Block*> m_BlockMap;

    // Spawn points for entities
    std::vector<SpawnPoint> m_SpawnPoints;

    // Lookup tables
    std::unordered_map<int, BlockDef> m_BlockDefs;
    std::unordered_map<int, EntityDef> m_EntityDefs;
    std::unordered_map<std::string, int> m_EntityNameToID;

    // Player spawn position
    float m_PlayerSpawnX = 3.0f * GameConfig::TILE_SIZE;
    float m_PlayerSpawnY = 12.0f * GameConfig::TILE_SIZE;
    float m_PipeExitX = 0.0f;

    // Static "empty" definitions for lookup failures
    static const BlockDef EMPTY_BLOCK_DEF;
    static const EntityDef EMPTY_ENTITY_DEF;

    // Grid to map key conversion
    int GridKey(int x, int y) const { return y * 10000 + x; }
};

}  // namespace Mario

#endif  // MARIO_LEVEL_HPP
