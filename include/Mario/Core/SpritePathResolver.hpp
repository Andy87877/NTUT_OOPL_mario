/**
 * @file SpritePathResolver.hpp
 * @brief Strategy pattern interfaces and utility to resolve sprite paths.
 *        Decouples block, player, entity, and castle sprite path resolution
 *        into separate polymorphic resolvers to improve extensibility (OCP/OOP).
 * @inheritance IBlockResolver, IPlayerResolver, IEntityResolver, ICastleResolver
 */
#ifndef MARIO_SPRITE_PATH_RESOLVER_HPP
#define MARIO_SPRITE_PATH_RESOLVER_HPP

#include <string>
#include <memory>

namespace Mario {

/**
 * @class IBlockResolver
 * @brief Strategy interface for resolving block sprite paths.
 * @inheritance None
 */
class IBlockResolver {
   public:
    virtual ~IBlockResolver() = default;
    virtual std::string Resolve(const std::string& blockName, int frame,
                                const std::string& levelName) = 0;
};

/**
 * @class IPlayerResolver
 * @brief Strategy interface for resolving player sprite paths.
 * @inheritance None
 */
class IPlayerResolver {
   public:
    virtual ~IPlayerResolver() = default;
    virtual std::string Resolve(const std::string& prefix, int state, int frame,
                                int starState) = 0;
};

/**
 * @class IEntityResolver
 * @brief Strategy interface for resolving entity sprite paths.
 * @inheritance None
 */
class IEntityResolver {
   public:
    virtual ~IEntityResolver() = default;
    virtual std::string Resolve(const std::string& entityName, int frame,
                                const std::string& levelName) = 0;
};

/**
 * @class ICastleResolver
 * @brief Strategy interface for resolving 8-4 castle sprite paths.
 * @inheritance None
 */
class ICastleResolver {
   public:
    virtual ~ICastleResolver() = default;
    virtual std::string Resolve(int blockID) = 0;
};

/**
 * @class SpritePathResolver
 * @brief Service class that delegates sprite path resolution to Strategy resolvers.
 *        Implements caching to avoid redundant disk I/O.
 * @inheritance None
 */
class SpritePathResolver {
   public:
    static void SetBlockResolver(std::unique_ptr<IBlockResolver> resolver);
    static void SetPlayerResolver(std::unique_ptr<IPlayerResolver> resolver);
    static void SetEntityResolver(std::unique_ptr<IEntityResolver> resolver);
    static void SetCastleResolver(std::unique_ptr<ICastleResolver> resolver);

    static std::string GetSpritePath(const std::string& name,
                                     const std::string& suffix = "");

    static std::string GetBlockSpritePath(const std::string& blockName,
                                          int frame = 0,
                                          const std::string& levelName = "");

    static std::string GetPlayerSpritePath(const std::string& prefix, int state,
                                           int frame, int starState = 0);

    static std::string GetEntitySpritePath(
        const std::string& entityName, int frame = 0,
        const std::string& levelName = "1-1");

    static std::string GetCastleSpritePathByID(int blockID);

    static const std::string SPRITE_BASE_PATH;

   private:
    static void EnsureResolversInitialized();

    static std::unique_ptr<IBlockResolver> s_BlockResolver;
    static std::unique_ptr<IPlayerResolver> s_PlayerResolver;
    static std::unique_ptr<IEntityResolver> s_EntityResolver;
    static std::unique_ptr<ICastleResolver> s_CastleResolver;
};

}  // namespace Mario

#endif  // MARIO_SPRITE_PATH_RESOLVER_HPP
