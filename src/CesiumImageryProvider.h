#ifndef _CESIUM_IMAGERY_PROVIDER_
#define _CESIUM_IMAGERY_PROVIDER_

#include <Cesium3DTilesSelection/Tileset.h>
#include <CesiumAsync/IAssetResponse.h>
#include <CesiumRasterOverlays/DebugColorizeTilesRasterOverlay.h>
#include <CesiumRasterOverlays/TileMapServiceRasterOverlay.h>
#include <CesiumUtility/IntrusivePointer.h>
#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;
using namespace Cesium3DTilesSelection;
using namespace CesiumRasterOverlays;
class RasterOverlayOptions;

namespace CesiumRasterOverlays
{
    class DebugColorizeTilesRasterOverlay;
}

namespace CesiumForGodot
{
    class ImageryProvider : public Resource
    {
        GDCLASS( ImageryProvider, Resource )

    protected:
        CesiumUtility::IntrusivePointer<CesiumRasterOverlays::TileMapServiceRasterOverlay>
            _p_overlay;
        String material_key;
        static void _bind_methods();

    public:
        ImageryProvider();
        ~ImageryProvider();

        String get_material_key() const;
        void set_material_key( const String p_material_key );
        virtual void addToTileset( Cesium3DTilesSelection::Tileset *p_tileset )
        {
            UtilityFunctions::print_verbose( "ImageryProvider::addToTileset" );
        }
        virtual void removeFromTileset( Cesium3DTilesSelection::Tileset *p_tileset )
        {
            UtilityFunctions::print_verbose( "ImageryProvider::removeFromTileset" );
        }
    };

    class GTileMapServiceRasterOverlay : public ImageryProvider
    {
        GDCLASS( GTileMapServiceRasterOverlay, ImageryProvider )

    private:
        String url;
        float maximum_screen_space_error;
        unsigned int maximum_texture_size;
        unsigned int maximum_simultaneous_tile_loads;
        unsigned int sub_tile_cache_bytes;
        bool specify_zoom_levels;
        unsigned int minimum_level;
        unsigned int maximum_level;

    protected:
        static void _bind_methods();

    public:
        GTileMapServiceRasterOverlay();
        ~GTileMapServiceRasterOverlay();

        void addToTileset( Cesium3DTilesSelection::Tileset *p_tileset ) override;
        void removeFromTileset( Cesium3DTilesSelection::Tileset *p_tileset ) override;

        float get_maximum_screen_space_error() const;
        void set_maximum_screen_space_error( const float p_maximum_screen_space_error );

        unsigned int get_maximum_texture_size() const;
        void set_maximum_texture_size( const unsigned int p_maximum_texture_size );

        unsigned int get_maximum_simultaneous_tile_loads() const;
        void set_maximum_simultaneous_tile_loads(
            const unsigned int p_maximum_simultaneous_tile_loads );

        unsigned int get_sub_tile_cache_bytes() const;
        void set_sub_tile_cache_bytes( const unsigned int p_sub_tile_cache_bytes );

        void set_url( const String p_url );
        String get_url() const;

        bool get_specify_zoom_levels() const;
        void set_specify_zoom_levels( const bool specify_zoom_levels );
        unsigned int get_minimum_level() const;
        void set_minimum_level( const unsigned int p_minimumLevel );
        unsigned int get_maximum_level() const;
        void set_maximum_level( const unsigned int p_maximumLevel );
    };

    class GDebugColorizeTilesRasterOverlay : public ImageryProvider
    {
        GDCLASS( GDebugColorizeTilesRasterOverlay, ImageryProvider )
    public:
        GDebugColorizeTilesRasterOverlay();
        ~GDebugColorizeTilesRasterOverlay();
        void addToTileset( Cesium3DTilesSelection::Tileset *p_tileset ) override;
        void removeFromTileset( Cesium3DTilesSelection::Tileset *p_tileset ) override;
    protected:
        static void _bind_methods();
    private:
        CesiumUtility::IntrusivePointer<CesiumRasterOverlays::DebugColorizeTilesRasterOverlay>
            _p_overlay;
    };
}

#endif