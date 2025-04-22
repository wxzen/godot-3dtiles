#include "GodotTilesetExternals.h"
#include "GodotAssetAccessor.h"
#include "GodotPrepareRendererResources.h"
#include "GodotTaskProcessor.h"
#include <CesiumAsync/CachingAssetAccessor.h>
#include <CesiumAsync/GunzipAssetAccessor.h>
#include <CesiumAsync/SqliteCache.h>
#include <CesiumUtility/CreditSystem.h>

using namespace CesiumUtility;
using namespace Cesium3DTilesSelection;
using namespace CesiumAsync;
using namespace Cesium3DTilesSelection;
using namespace godot;

namespace CesiumForGodot
{
    namespace
    {
        std::shared_ptr<IAssetAccessor> pAccessor = nullptr;
        std::shared_ptr<ITaskProcessor> pTaskProcessor = nullptr;
        std::shared_ptr<CreditSystem> pCreditSystem = nullptr;
        std::optional<AsyncSystem> asyncSystem;
    } // namespace

    const std::shared_ptr<IAssetAccessor> &getAssetAccessor()
    {
        if ( !pAccessor )
        {
            std::string cacheDBPath = "./cesium-request-cache.sqlite";
            int32_t requestsPerCachePrune = 100;
            uint64_t maxItems = 1000;

            pAccessor =
                std::make_shared<GunzipAssetAccessor>( std::make_shared<CachingAssetAccessor>(
                    spdlog::default_logger(), std::make_shared<GodotAssetAccessor>(),
                    std::make_shared<SqliteCache>( spdlog::default_logger(), cacheDBPath,
                                                   maxItems ),
                    requestsPerCachePrune ) );
        }
        return pAccessor;
    }

    const std::shared_ptr<ITaskProcessor> &getTaskProcessor()
    {
        if ( !pTaskProcessor )
        {
            pTaskProcessor = std::make_shared<GodotTaskProcessor>();
        }
        return pTaskProcessor;
    }

    AsyncSystem getAsyncSystem()
    {
        if ( !asyncSystem )
        {
            asyncSystem.emplace( getTaskProcessor() );
        }
        return *asyncSystem;
    }

    const std::shared_ptr<CreditSystem> &getOrCreateCreditSystem( Cesium3DTileset *tileset )
    {
        return pCreditSystem;
    }

    Cesium3DTilesSelection::TilesetExternals createTilesetExternals( Cesium3DTileset *tileset )
    {
        return TilesetExternals{ getAssetAccessor(),
                                 std::make_shared<GodotPrepareRendererResources>( tileset ),
                                 AsyncSystem( getTaskProcessor() ),
                                 getOrCreateCreditSystem( tileset ), spdlog::default_logger() };
    }

} // namespace CesiumForGodot