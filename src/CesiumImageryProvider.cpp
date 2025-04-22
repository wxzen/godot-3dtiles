#include "CesiumImageryProvider.h"
#include "Cesium3DTileset.h"
#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/variant/string.hpp>

using namespace CesiumForGodot;

void ImageryProvider::_bind_methods()
{

    ClassDB::bind_method( D_METHOD( "get_material_key" ), &ImageryProvider::get_material_key );
    ClassDB::bind_method( D_METHOD( "set_material_key", "p_material_key" ),
                          &ImageryProvider::set_material_key );
    ADD_PROPERTY(
        PropertyInfo( Variant::STRING, "material_key", PROPERTY_HINT_ENUM, "0, 1, 2, Clipping" ),
        "set_material_key", "get_material_key" );
}

ImageryProvider::ImageryProvider() : material_key( "0" )
{
}

ImageryProvider::~ImageryProvider()
{
}

String ImageryProvider::get_material_key() const
{
    return this->material_key;
}

void ImageryProvider::set_material_key( const String p_material_key )
{
    if ( this->material_key != p_material_key )
    {
        this->material_key = p_material_key;
    }
}

void GTileMapServiceRasterOverlay::_bind_methods()
{
    ClassDB::bind_method( D_METHOD( "get_url" ), &GTileMapServiceRasterOverlay::get_url );
    ClassDB::bind_method( D_METHOD( "set_url", "p_url" ), &GTileMapServiceRasterOverlay::set_url );
    ADD_PROPERTY( PropertyInfo( Variant::STRING, "url" ), "set_url", "get_url" );

    ClassDB::bind_method( D_METHOD( "get_maximum_screen_space_error" ),
                          &GTileMapServiceRasterOverlay::get_maximum_screen_space_error );
    ClassDB::bind_method(
        D_METHOD( "set_maximum_screen_space_error", "p_maximum_screen_space_error" ),
        &GTileMapServiceRasterOverlay::set_maximum_screen_space_error );
    ADD_PROPERTY( PropertyInfo( Variant::INT, "maximum_screen_space_error" ),
                  "set_maximum_screen_space_error", "get_maximum_screen_space_error" );

    ClassDB::bind_method( D_METHOD( "get_maximum_texture_size" ),
                          &GTileMapServiceRasterOverlay::get_maximum_texture_size );
    ClassDB::bind_method( D_METHOD( "set_maximum_texture_size", "p_maximum_texture_size" ),
                          &GTileMapServiceRasterOverlay::set_maximum_texture_size );
    ADD_PROPERTY( PropertyInfo( Variant::INT, "maximum_texture_size" ), "set_maximum_texture_size",
                  "get_maximum_texture_size" );

    ClassDB::bind_method( D_METHOD( "get_maximum_simultaneous_tile_loads" ),
                          &GTileMapServiceRasterOverlay::get_maximum_simultaneous_tile_loads );
    ClassDB::bind_method(
        D_METHOD( "set_maximum_simultaneous_tile_loads", "p_maximum_simultaneous_tile_loads" ),
        &GTileMapServiceRasterOverlay::set_maximum_simultaneous_tile_loads );
    ADD_PROPERTY( PropertyInfo( Variant::INT, "maximum_simultaneous_tile_loads" ),
                  "set_maximum_simultaneous_tile_loads", "get_maximum_simultaneous_tile_loads" );

    ClassDB::bind_method( D_METHOD( "get_sub_tile_cache_bytes" ),
                          &GTileMapServiceRasterOverlay::get_sub_tile_cache_bytes );
    ClassDB::bind_method( D_METHOD( "set_sub_tile_cache_bytes", "p_sub_tile_cache_bytes" ),
                          &GTileMapServiceRasterOverlay::set_sub_tile_cache_bytes );
    ADD_PROPERTY( PropertyInfo( Variant::INT, "sub_tile_cache_bytes" ), "set_sub_tile_cache_bytes",
                  "get_sub_tile_cache_bytes" );

    ClassDB::bind_method( D_METHOD( "get_specify_zoom_levels" ),
                          &GTileMapServiceRasterOverlay::get_specify_zoom_levels );
    ClassDB::bind_method( D_METHOD( "set_specify_zoom_levels", "p_specify_zoom_levels" ),
                          &GTileMapServiceRasterOverlay::set_specify_zoom_levels );
    ADD_PROPERTY( PropertyInfo( Variant::BOOL, "specify_zoom_levels" ), "set_specify_zoom_levels",
                  "get_specify_zoom_levels" );

    ClassDB::bind_method( D_METHOD( "get_minimum_level" ),
                          &GTileMapServiceRasterOverlay::get_minimum_level );
    ClassDB::bind_method( D_METHOD( "set_minimum_level", "p_minimum_level" ),
                          &GTileMapServiceRasterOverlay::set_minimum_level );
    ADD_PROPERTY( PropertyInfo( Variant::INT, "minimum_level" ), "set_minimum_level",
                  "get_minimum_level" );

    ClassDB::bind_method( D_METHOD( "get_maximum_level" ),
                          &GTileMapServiceRasterOverlay::get_maximum_level );
    ClassDB::bind_method( D_METHOD( "set_maximum_level", "p_maximumLevel" ),
                          &GTileMapServiceRasterOverlay::set_maximum_level );
    ADD_PROPERTY( PropertyInfo( Variant::INT, "maximumLevel" ), "set_maximum_level",
                  "get_maximum_level" );
}

GTileMapServiceRasterOverlay::GTileMapServiceRasterOverlay() :
    url( "" ), maximum_screen_space_error( 2.0f ), maximum_texture_size( 2048 ),
    maximum_simultaneous_tile_loads( 20 ), sub_tile_cache_bytes( 16 * 1024 * 1024 ),
    specify_zoom_levels( false ), minimum_level( 0 ), maximum_level( 10 ) {};

GTileMapServiceRasterOverlay::~GTileMapServiceRasterOverlay()
{
}

void GTileMapServiceRasterOverlay::addToTileset( Cesium3DTilesSelection::Tileset *p_tileset )
{
    if ( this->_p_overlay != nullptr )
    {
        return;
    }

    if ( this->url.is_empty() )
    {
        // Don't create an overlay with an empty URL.
        return;
    }

    if ( !p_tileset )
    {
        return;
    }

    TileMapServiceRasterOverlayOptions tmsOptions;
    if ( this->maximum_level > this->minimum_level && this->specify_zoom_levels )
    {
        tmsOptions.minimumLevel = this->minimum_level;
        tmsOptions.maximumLevel = this->maximum_level;
    }

    CesiumRasterOverlays::RasterOverlayOptions options{};
    options.maximumScreenSpaceError = this->maximum_screen_space_error;
    options.maximumSimultaneousTileLoads = this->maximum_simultaneous_tile_loads;
    options.maximumTextureSize = this->maximum_texture_size;
    options.subTileCacheBytes = this->sub_tile_cache_bytes;
    options.showCreditsOnScreen = false;
    options.loadErrorCallback =
        []( const CesiumRasterOverlays::RasterOverlayLoadFailureDetails &details ) {
            int typeValue = (int)details.type;
            int64_t statusCode =
                details.pRequest && details.pRequest->response()
                    ? static_cast<int64_t>( details.pRequest->response()->statusCode() )
                    : 0;
            UtilityFunctions::printerr( "status code->", statusCode, ", message->",
                                        StringName( details.message.c_str() ) );
        };

    this->_p_overlay = new TileMapServiceRasterOverlay(
        this->material_key.utf8().get_data(), url.utf8().get_data(),
        std::vector<CesiumAsync::IAssetAccessor::THeader>(), tmsOptions, options );

    p_tileset->getOverlays().add( this->_p_overlay );
}

void GTileMapServiceRasterOverlay::removeFromTileset( Cesium3DTilesSelection::Tileset *p_tileset )
{
    if ( this->_p_overlay == nullptr )
    {
        return;
    }

    if ( !p_tileset )
    {
        return;
    }

    p_tileset->getOverlays().remove( this->_p_overlay );
    this->_p_overlay = nullptr;
}

String GTileMapServiceRasterOverlay::get_url() const
{
    return this->url;
}

void GTileMapServiceRasterOverlay::set_url( const String p_url )
{
    if ( this->url != p_url )
    {
        this->url = p_url;
    }
}

float GTileMapServiceRasterOverlay::get_maximum_screen_space_error() const
{
    return this->maximum_screen_space_error;
}

void GTileMapServiceRasterOverlay::set_maximum_screen_space_error(
    const float p_maximum_screen_space_error )
{
    if ( this->maximum_screen_space_error != p_maximum_screen_space_error )
    {
        this->maximum_screen_space_error = p_maximum_screen_space_error;
    }
}

unsigned int GTileMapServiceRasterOverlay::get_maximum_texture_size() const
{
    return this->maximum_texture_size;
}

void GTileMapServiceRasterOverlay::set_maximum_texture_size(
    const unsigned int p_maximum_texture_size )
{
    if ( this->maximum_texture_size != p_maximum_texture_size )
    {
        this->maximum_texture_size = p_maximum_texture_size;
    }
}

unsigned int GTileMapServiceRasterOverlay::get_maximum_simultaneous_tile_loads() const
{
    return this->maximum_simultaneous_tile_loads;
}

void GTileMapServiceRasterOverlay::set_maximum_simultaneous_tile_loads(
    const unsigned int p_maximum_simultaneous_tile_loads )
{
    if ( this->maximum_simultaneous_tile_loads != p_maximum_simultaneous_tile_loads )
    {
        this->maximum_simultaneous_tile_loads = p_maximum_simultaneous_tile_loads;
    }
}

unsigned int GTileMapServiceRasterOverlay::get_sub_tile_cache_bytes() const
{
    return this->sub_tile_cache_bytes;
}

void GTileMapServiceRasterOverlay::set_sub_tile_cache_bytes(
    const unsigned int p_sub_tile_cache_bytes )
{
    if ( this->sub_tile_cache_bytes != p_sub_tile_cache_bytes )
    {
        this->sub_tile_cache_bytes = p_sub_tile_cache_bytes;
    }
}
bool GTileMapServiceRasterOverlay::get_specify_zoom_levels() const
{
    return this->specify_zoom_levels;
}

void GTileMapServiceRasterOverlay::set_specify_zoom_levels( const bool p_specify_zoom_levels )
{
    if ( this->specify_zoom_levels != p_specify_zoom_levels )
    {
        this->specify_zoom_levels = p_specify_zoom_levels;
    }
}

unsigned int GTileMapServiceRasterOverlay::get_minimum_level() const
{
    return this->minimum_level;
}

void GTileMapServiceRasterOverlay::set_minimum_level( const unsigned int p_minimum_level )
{
    if ( this->minimum_level != p_minimum_level )
    {
        this->minimum_level = p_minimum_level;
    }
}

unsigned int GTileMapServiceRasterOverlay::get_maximum_level() const
{
    return this->maximum_level;
}

void GTileMapServiceRasterOverlay::set_maximum_level( const unsigned int p_maximumLevel )
{
    if ( this->maximum_level != p_maximumLevel )
    {
        this->maximum_level = p_maximumLevel;
    }
}

GDebugColorizeTilesRasterOverlay::GDebugColorizeTilesRasterOverlay() : _p_overlay( nullptr )
{
}

GDebugColorizeTilesRasterOverlay::~GDebugColorizeTilesRasterOverlay()
{
}

void GDebugColorizeTilesRasterOverlay::_bind_methods()
{
}

void GDebugColorizeTilesRasterOverlay::addToTileset( Cesium3DTilesSelection::Tileset *p_tileset )
{
    if ( this->_p_overlay != nullptr )
    {
        return;
    }
    if ( !p_tileset )
    {
        return;
    }
    CesiumRasterOverlays::RasterOverlayOptions options{};
    options.loadErrorCallback =
        []( const CesiumRasterOverlays::RasterOverlayLoadFailureDetails &details ) {
            int typeValue = (int)details.type;
            int64_t statusCode =
                details.pRequest && details.pRequest->response()
                    ? static_cast<int64_t>( details.pRequest->response()->statusCode() )
                    : 0;
            UtilityFunctions::printerr( "status code->", statusCode, ", message->",
                                        StringName( details.message.c_str() ) );
        };

    this->_p_overlay = new CesiumRasterOverlays::DebugColorizeTilesRasterOverlay(
        this->material_key.utf8().get_data(), options );

    p_tileset->getOverlays().add( this->_p_overlay );
}

void GDebugColorizeTilesRasterOverlay::removeFromTileset(
    Cesium3DTilesSelection::Tileset *p_tileset )
{
    if ( this->_p_overlay == nullptr )
    {
        return;
    }

    if ( !p_tileset )
    {
        return;
    }

    p_tileset->getOverlays().remove( this->_p_overlay );
    this->_p_overlay = nullptr;
}