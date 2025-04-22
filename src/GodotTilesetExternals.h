#ifndef GODOT_TILESET_EXTERNALS_H
#define GODOT_TILESET_EXTERNALS_H

#include "Cesium3DTileset.h"
#include <Cesium3DTilesSelection/TilesetExternals.h>
#include <memory>

namespace Cesium3DTilesSelection
{
    class TilesetExternals;
} // namespace Cesium3DTilesSelection

namespace CesiumUtility
{
    class CreditSystem;
} // namespace CesiumUtility

namespace CesiumAsync
{
    class AsyncSystem;
    class IAssetAccessor;
    class ITaskProcessor;
} // namespace CesiumAsync

namespace CesiumForGodot
{
    const std::shared_ptr<CesiumAsync::IAssetAccessor> &getAssetAccessor();
    const std::shared_ptr<CesiumAsync::ITaskProcessor> &getTaskProcessor();
    CesiumAsync::AsyncSystem getAsyncSystem();

    // Gets the credit system on the input Cesium3DTileset. If it does not exist,
    // this will create a new default credit system for the tileset.
    const std::shared_ptr<CesiumUtility::CreditSystem> &getOrCreateCreditSystem(
        Cesium3DTileset *tileset );

    Cesium3DTilesSelection::TilesetExternals createTilesetExternals( Cesium3DTileset *tileset );
} // namespace CesiumForGodot

#endif