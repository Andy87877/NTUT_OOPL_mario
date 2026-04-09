/**
 * @file Level.cpp
 * @brief Implementation of Level CSV parsing and block grid management.
 *        Matches the C# reference's Form1.cs LoadLevel() logic.
 * @inheritance None
 */
#include "Mario/Level.hpp"

#include <algorithm>
#include <fstream>
#include <sstream>

#include "Mario/SpritePathResolver.hpp"
#include "Util/Logger.hpp"

namespace Mario {

// Static empty definitions
const BlockDef Level::EMPTY_BLOCK_DEF = {};
const EntityDef Level::EMPTY_ENTITY_DEF = {};

Level::Level() { LoadLookupTables(); }

bool Level::Load(const std::string& levelName) {
    m_LevelName = levelName;
    m_Blocks.clear();
    m_BlockMap.clear();
    m_SpawnPoints.clear();
    m_Grid.clear();

    // Determine sub-level name
    if (levelName == "1-1") {
        m_SubLevelName = "1-1u";  // Underground bonus area
    } else if (levelName == "1-2") {
        m_SubLevelName = "1-2u";  // 1-2 is underground -> above ground exit
    } else {
        m_SubLevelName = "";
    }

    std::string path =
        std::string(RESOURCE_DIR) + "/Levels/" + levelName + ".csv";
    if (!ParseLevelCSV(path)) {
        LOG_ERROR("Failed to parse level CSV: {}", path);
        return false;
    }

    CreateBlocksFromGrid();
    IdentifySpawnPoints();

    // For 8-4 level, always override spawn point to avoid player spawning in
    // blocks Bowser's Castle requires specific spawn positioning regardless of
    // CSV markers
    if (levelName == "8-4") {
        // Override spawn point: start at left side (gridX=3), well above blocks
        // (gridY=3) to avoid spawning inside castle blocks
        m_PlayerSpawnX = 3.0f * GameConfig::TILE_SIZE;
        m_PlayerSpawnY = 3.0f * GameConfig::TILE_SIZE;
    }

    LOG_INFO("Level {} loaded: {}x{} tiles, {} blocks, {} spawn points",
             levelName, m_Width, m_Height, m_Blocks.size(),
             m_SpawnPoints.size());

    return true;
}

bool Level::ParseLevelCSV(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        LOG_ERROR("Cannot open level file: {}", path);
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        // Remove \r if present (Windows line endings)
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }

        auto tokens = SplitCSVLine(line);
        std::vector<int> row;
        row.reserve(tokens.size());

        for (const auto& token : tokens) {
            if (token.empty()) {
                row.push_back(0);
            } else {
                try {
                    row.push_back(std::stoi(token));
                } catch (...) {
                    row.push_back(0);
                }
            }
        }
        m_Grid.push_back(row);
    }

    m_Height = static_cast<int>(m_Grid.size());
    m_Width = 0;
    for (const auto& row : m_Grid) {
        m_Width = std::max(m_Width, static_cast<int>(row.size()));
    }

    return m_Height > 0 && m_Width > 0;
}

void Level::LoadLookupTables() {
    // Load IDList.csv (block definitions)
    {
        std::string path =
            std::string(RESOURCE_DIR) + "/LookUpSheet/IDList.csv";
        std::ifstream file(path);
        if (!file.is_open()) {
            LOG_ERROR("Cannot open IDList.csv: {}", path);
            return;
        }

        std::string line;
        while (std::getline(file, line)) {
            if (!line.empty() && line.back() == '\r') line.pop_back();
            auto tokens = SplitCSVLine(line);
            if (tokens.size() < 2 || tokens[1].empty()) continue;

            BlockDef def;
            def.id = std::stoi(tokens[0]);
            def.name = tokens[1];
            if (tokens.size() > 2) def.solid = (tokens[2] == "1");
            if (tokens.size() > 3) def.breakable = (tokens[3] == "1");
            if (tokens.size() > 4) def.background = (tokens[4] == "1");
            if (tokens.size() > 5) def.isGoal = (tokens[5] == "1");
            if (tokens.size() > 6)
                def.contentID = tokens[6].empty() ? 0 : std::stoi(tokens[6]);
            if (tokens.size() > 7)
                def.r = tokens[7].empty() ? 0 : std::stoi(tokens[7]);
            if (tokens.size() > 8)
                def.g = tokens[8].empty() ? 0 : std::stoi(tokens[8]);
            if (tokens.size() > 9)
                def.b = tokens[9].empty() ? 0 : std::stoi(tokens[9]);
            if (tokens.size() > 10) def.isContainer = (tokens[10] == "1");
            if (tokens.size() > 11) def.contents = tokens[11];
            if (tokens.size() > 12) def.hitSpriteName = tokens[12];
            if (tokens.size() > 13) def.animated = (tokens[13] == "1");
            if (tokens.size() > 14)
                def.animationFrames =
                    tokens[14].empty() ? 0 : std::stoi(tokens[14]);
            if (tokens.size() > 15) def.bounceBack = (tokens[15] == "1");
            if (tokens.size() > 16) def.spawner = (tokens[16] == "1");
            if (tokens.size() > 17) def.spawnEntity = tokens[17];

            m_BlockDefs[def.id] = def;
        }
        LOG_INFO("Loaded {} block definitions", m_BlockDefs.size());
    }

    // Load EntityList.csv (entity definitions)
    {
        std::string path =
            std::string(RESOURCE_DIR) + "/LookUpSheet/EntityList.csv";
        std::ifstream file(path);
        if (!file.is_open()) {
            LOG_ERROR("Cannot open EntityList.csv: {}", path);
            return;
        }

        std::string line;
        while (std::getline(file, line)) {
            if (!line.empty() && line.back() == '\r') line.pop_back();
            auto tokens = SplitCSVLine(line);
            if (tokens.size() < 2 || tokens[1].empty()) continue;

            EntityDef def;
            def.id = std::stoi(tokens[0]);
            def.name = tokens[1];
            if (tokens.size() > 2) def.isPowerUp = (tokens[2] == "1");
            if (tokens.size() > 3) def.isEnemy = (tokens[3] == "1");
            if (tokens.size() > 4) def.isCoin = (tokens[4] == "1");
            if (tokens.size() > 5)
                def.powerUpState = tokens[5].empty() ? 0 : std::stoi(tokens[5]);
            if (tokens.size() > 6) def.isStatic = (tokens[6] == "1");
            if (tokens.size() > 7) def.isBounce = (tokens[7] == "1");
            if (tokens.size() > 8) def.fromBlock = (tokens[8] == "1");
            if (tokens.size() > 9)
                def.scoreWorth = tokens[9].empty() ? 0 : std::stoi(tokens[9]);
            if (tokens.size() > 10) def.isAnimated = (tokens[10] == "1");
            if (tokens.size() > 11)
                def.animFrames = tokens[11].empty() ? 0 : std::stoi(tokens[11]);
            if (tokens.size() > 12) def.doesJump = (tokens[12] == "1");
            if (tokens.size() > 13) def.doesCollide = (tokens[13] == "1");
            if (tokens.size() > 14) def.oneLoop = (tokens[14] == "1");
            if (tokens.size() > 15)
                def.animBuffer = tokens[15].empty() ? 1 : std::stoi(tokens[15]);
            if (tokens.size() > 16) def.squishable = (tokens[16] == "1");
            if (tokens.size() > 17) def.koopaSquash = (tokens[17] == "1");

            m_EntityDefs[def.id] = def;
            m_EntityNameToID[def.name] = def.id;
        }
        LOG_INFO("Loaded {} entity definitions", m_EntityDefs.size());
    }
}

void Level::CreateBlocksFromGrid() {
    for (int y = 0; y < m_Height; y++) {
        for (int x = 0; x < static_cast<int>(m_Grid[y].size()); x++) {
            int blockID = m_Grid[y][x];

            // Skip empty cells
            if (blockID == 0 && m_BlockDefs.find(0) == m_BlockDefs.end())
                continue;

            // Skip empty/undefined blocks
            auto it = m_BlockDefs.find(blockID);
            if (it == m_BlockDefs.end()) continue;
            if (it->second.name.empty()) continue;

            // Skip sky (ID 0) - we use background color
            if (blockID == 0) continue;

            // Check for special IDs (matching C# Form1.cs StartLevel())
            // 997/998/999 = Mario spawn, 897/898/899 = Luigi spawn (unused)
            if (blockID == 997 || blockID == 998 || blockID == 999) {
                m_PlayerSpawnX = static_cast<float>(x * GameConfig::TILE_SIZE);
                m_PlayerSpawnY = static_cast<float>(y * GameConfig::TILE_SIZE);
                continue;
            }
            if (blockID == 897 || blockID == 898 || blockID == 899) {
                // Luigi spawn - skip for single-player mode
                continue;
            }

            // Flag entity (ID 29) - track for ending sequence
            if (blockID == 29) {
                SpawnPoint sp;
                sp.entityName = "Flag";
                sp.entityID = 29;
                sp.gridX = x;
                sp.gridY = y;
                sp.worldX = static_cast<float>(x * GameConfig::TILE_SIZE) +
                            GameConfig::TILE_SIZE * 0.5f;
                sp.worldY = static_cast<float>(y * GameConfig::TILE_SIZE);
                m_SpawnPoints.push_back(sp);
                continue;
            }

            // UnderCoin (ID 49) - coin entity below surface
            if (blockID == 49) {
                SpawnPoint sp;
                sp.entityName = "UnderCoin";
                sp.entityID = 49;
                sp.gridX = x;
                sp.gridY = y;
                sp.worldX = static_cast<float>(x * GameConfig::TILE_SIZE);
                sp.worldY = static_cast<float>(y * GameConfig::TILE_SIZE +
                                               GameConfig::TILE_SIZE);
                m_SpawnPoints.push_back(sp);
                continue;
            }

            // Pipe exit marker (ID 53) - record position but still render
            if (blockID == 53) {
                m_PipeExitX = static_cast<float>(x * GameConfig::TILE_SIZE) -
                              3.0f * GameConfig::TILE_SIZE;
                // Don't continue — this is also a visible Pipe3 block
            }

            // Entity spawn points (via block spawner flag)
            if (it->second.spawner && !it->second.spawnEntity.empty()) {
                SpawnPoint sp;
                sp.entityName = it->second.spawnEntity;
                sp.gridX = x;
                sp.gridY = y;
                sp.worldX = static_cast<float>(x * GameConfig::TILE_SIZE);
                sp.worldY = static_cast<float>(y * GameConfig::TILE_SIZE);

                auto eit = m_EntityNameToID.find(sp.entityName);
                if (eit != m_EntityNameToID.end()) {
                    sp.entityID = eit->second;
                }

                m_SpawnPoints.push_back(sp);
                continue;
            }

            // Create the block
            const BlockDef& def = it->second;
            auto block = std::make_shared<Block>(blockID, x, y, def);
            m_Blocks.push_back(block);
            m_BlockMap[GridKey(x, y)] = block.get();
        }
    }
}

void Level::IdentifySpawnPoints() {
    /**
     * Scan grid and identify all entity spawn points.
     * Supports both hardcoded legacy IDs (50=Goomba, 66=Koopa) and
     * BlockDef-based spawner flags (compatible with 8-4 level IDs).
     *
     * For 8-4 (800+ ID range):
     *   - ID 882 (800+82) = Goomba if GoombaSpawn is defined
     *   - ID 886 (800+86) = Koopa if KoopaSpawn is defined
     */
    for (int y = 0; y < m_Height; y++) {
        for (int x = 0; x < static_cast<int>(m_Grid[y].size()); x++) {
            int blockID = m_Grid[y][x];

            // Look up block definition for this ID
            auto it = m_BlockDefs.find(blockID);
            if (it != m_BlockDefs.end() && it->second.spawner &&
                !it->second.spawnEntity.empty()) {
                // Block definition says this tile spawns an entity
                SpawnPoint sp;
                sp.entityName = it->second.spawnEntity;
                sp.gridX = x;
                sp.gridY = y;
                sp.worldX = static_cast<float>(x * GameConfig::TILE_SIZE);
                sp.worldY = static_cast<float>(y * GameConfig::TILE_SIZE);

                // Look up entity ID from name
                auto eit = m_EntityNameToID.find(sp.entityName);
                if (eit != m_EntityNameToID.end()) {
                    sp.entityID = eit->second;
                }

                m_SpawnPoints.push_back(sp);
                continue;
            }

            // Legacy hardcoded enemy spawn markers (supports 1-1, 1-2)
            if (blockID == 50) {  // GoombaSpawn (ID list defined)
                SpawnPoint sp;
                sp.entityName = "Goomba";
                sp.entityID = 13;
                sp.gridX = x;
                sp.gridY = y;
                sp.worldX = static_cast<float>(x * GameConfig::TILE_SIZE);
                sp.worldY = static_cast<float>(y * GameConfig::TILE_SIZE);
                m_SpawnPoints.push_back(sp);
            } else if (blockID == 66) {  // KoopaSpawn (ID list defined)
                SpawnPoint sp;
                sp.entityName = "KoopaTroopa";
                sp.entityID = 17;
                sp.gridX = x;
                sp.gridY = y;
                sp.worldX = static_cast<float>(x * GameConfig::TILE_SIZE);
                sp.worldY = static_cast<float>(y * GameConfig::TILE_SIZE);
                m_SpawnPoints.push_back(sp);
            }
        }
    }
}

void Level::UpdateBlocks(float cameraOffset) {
    for (auto& block : m_Blocks) {
        // Only update visible blocks (performance optimization)
        float worldX = block->GetWorldX();
        float screenX = worldX - cameraOffset;

        bool visible =
            (screenX + GameConfig::TILE_SIZE > -GameConfig::TILE_SIZE * 2) &&
            (screenX < GameConfig::WINDOW_WIDTH + GameConfig::TILE_SIZE * 2);

        if (visible) {
            block->Update(cameraOffset);
        }
    }
}

Block* Level::GetBlockAt(int gridX, int gridY) {
    auto it = m_BlockMap.find(GridKey(gridX, gridY));
    return (it != m_BlockMap.end()) ? it->second : nullptr;
}

const Block* Level::GetBlockAt(int gridX, int gridY) const {
    auto it = m_BlockMap.find(GridKey(gridX, gridY));
    return (it != m_BlockMap.end()) ? it->second : nullptr;
}

Block* Level::GetBlockAtWorld(float worldX, float worldY) {
    int gridX = static_cast<int>(worldX) / GameConfig::TILE_SIZE;
    int gridY = static_cast<int>(worldY) / GameConfig::TILE_SIZE;
    return GetBlockAt(gridX, gridY);
}

std::vector<Block*> Level::GetBlocksInRange(float leftX, float rightX) const {
    std::vector<Block*> result;

    int startCol =
        std::max(0, static_cast<int>(leftX) / GameConfig::TILE_SIZE - 1);
    int endCol = std::min(m_Width - 1,
                          static_cast<int>(rightX) / GameConfig::TILE_SIZE + 1);

    for (int x = startCol; x <= endCol; x++) {
        for (int y = 0; y < m_Height; y++) {
            auto it = m_BlockMap.find(GridKey(x, y));
            if (it != m_BlockMap.end() && it->second != nullptr) {
                result.push_back(it->second);
            }
        }
    }
    return result;
}

const BlockDef& Level::GetBlockDef(int id) const {
    auto it = m_BlockDefs.find(id);
    return (it != m_BlockDefs.end()) ? it->second : EMPTY_BLOCK_DEF;
}

const EntityDef& Level::GetEntityDef(int id) const {
    auto it = m_EntityDefs.find(id);
    return (it != m_EntityDefs.end()) ? it->second : EMPTY_ENTITY_DEF;
}

const EntityDef& Level::GetEntityDefByName(const std::string& name) const {
    auto nit = m_EntityNameToID.find(name);
    if (nit != m_EntityNameToID.end()) {
        return GetEntityDef(nit->second);
    }
    return EMPTY_ENTITY_DEF;
}

std::vector<std::string> Level::SplitCSVLine(const std::string& line) const {
    std::vector<std::string> tokens;
    std::stringstream ss(line);
    std::string token;
    while (std::getline(ss, token, ',')) {
        tokens.push_back(token);
    }
    return tokens;
}

}  // namespace Mario
