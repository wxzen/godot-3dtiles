#ifndef CESIUM_TILE_EXCLUDER_H
#define CESIUM_TILE_EXCLUDER_H

namespace CesiumForGodot
{

    class Cesium3DTileset;

    class CesiumTileExcluder
    {
    public:
        static void addToTileset( Cesium3DTileset &tileset );
        static void removeFromTileset( Cesium3DTileset &tileset );
    };

} // namespace CesiumForGodot

#endif