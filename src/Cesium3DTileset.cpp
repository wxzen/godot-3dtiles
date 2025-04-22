#include "Cesium3DTileset.h"
#include "CameraManager.h"
#include "GodotPrepareRendererResources.h"
#include "GodotTilesetExternals.h"

#include <Cesium3DTilesSelection/EllipsoidTilesetLoader.h>
#include <Cesium3DTilesSelection/Tileset.h>
#include <CesiumGeospatial/GlobeTransforms.h>

#include <godot_cpp/classes/camera3d.hpp>
#include <godot_cpp/classes/editor_interface.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/sub_viewport.hpp>
#include <godot_cpp/classes/viewport.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/templates/vector.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;
using namespace CesiumForGodot;
using namespace Cesium3DTilesSelection;

void Cesium3DTileset::_bind_methods()
{
    ClassDB::bind_method( D_METHOD( "get_tileset_source" ), &Cesium3DTileset::get_tileset_source );
    ClassDB::bind_method( D_METHOD( "set_tileset_source", "p_tileset_source" ),
                          &Cesium3DTileset::set_tileset_source );
    ADD_PROPERTY( PropertyInfo( Variant::INT, "tileset_source", PROPERTY_HINT_ENUM,
                                "FromUrl, FromEllipsoid" ),
                  "set_tileset_source", "get_tileset_source" );

    ClassDB::bind_method( D_METHOD( "get_url" ), &Cesium3DTileset::get_url );
    ClassDB::bind_method( D_METHOD( "set_url", "p_url" ), &Cesium3DTileset::set_url );
    ADD_PROPERTY( PropertyInfo( Variant::STRING, "url" ), "set_url", "get_url" );

    // ClassDB::bind_method( D_METHOD( "get_test_array" ), &Cesium3DTileset::get_test_array );
    // ClassDB::bind_method( D_METHOD( "set_test_array", "p_test_array" ),
    //                       &Cesium3DTileset::set_test_array );
    // ADD_PROPERTY( PropertyInfo( Variant::ARRAY, "test_array", PROPERTY_HINT_ARRAY_TYPE ),
    //               "set_test_array", "get_test_array" );

    ClassDB::bind_method( D_METHOD( "get_maximum_screen_space_error" ),
                          &Cesium3DTileset::get_maximum_screen_space_error );
    ClassDB::bind_method(
        D_METHOD( "set_maximum_screen_space_error", "p_maximum_screen_space_error" ),
        &Cesium3DTileset::set_maximum_screen_space_error );
    ADD_PROPERTY( PropertyInfo( Variant::FLOAT, "maximum screenspace error" ),
                  "set_maximum_screen_space_error", "get_maximum_screen_space_error" );

    ClassDB::bind_method( D_METHOD( "get_preload_ancestors" ),
                          &Cesium3DTileset::get_preload_ancestors );
    ClassDB::bind_method( D_METHOD( "set_preload_ancestors", "p_preload_ancestors" ),
                          &Cesium3DTileset::set_preload_ancestors );
    ADD_PROPERTY( PropertyInfo( Variant::BOOL, "preload ancestors" ), "set_preload_ancestors",
                  "get_preload_ancestors" );

    ClassDB::bind_method( D_METHOD( "get_preload_siblings" ),
                          &Cesium3DTileset::get_preload_siblings );
    ClassDB::bind_method( D_METHOD( "set_preload_siblings", "p_preload_siblings" ),
                          &Cesium3DTileset::set_preload_siblings );
    ADD_PROPERTY( PropertyInfo( Variant::BOOL, "preload siblings" ), "set_preload_siblings",
                  "get_preload_siblings" );

    ClassDB::bind_method( D_METHOD( "get_forbid_holes" ), &Cesium3DTileset::get_forbid_holes );
    ClassDB::bind_method( D_METHOD( "set_forbid_holes", "p_forbid_holes" ),
                          &Cesium3DTileset::set_forbid_holes );
    ADD_PROPERTY( PropertyInfo( Variant::BOOL, "forbid holes" ), "set_forbid_holes",
                  "get_forbid_holes" );

    ClassDB::bind_method( D_METHOD( "get_maximum_simultaneous_tile_loads" ),
                          &Cesium3DTileset::get_maximum_simultaneous_tile_loads );
    ClassDB::bind_method(
        D_METHOD( "set_maximum_simultaneous_tile_loads", "p_maximum_simultaneous_tile_loads" ),
        &Cesium3DTileset::set_maximum_simultaneous_tile_loads );
    ADD_PROPERTY( PropertyInfo( Variant::INT, "maximum simultaneous tile loads" ),
                  "set_maximum_simultaneous_tile_loads", "get_maximum_simultaneous_tile_loads" );

    ClassDB::bind_method( D_METHOD( "get_maximum_cached_mbytes" ),
                          &Cesium3DTileset::get_maximum_cached_mbytes );
    ClassDB::bind_method( D_METHOD( "set_maximum_cached_mbytes", "p_maximum_cached_mbytes" ),
                          &Cesium3DTileset::set_maximum_cached_mbytes );
    ADD_PROPERTY( PropertyInfo( Variant::INT, "maximum cached MB" ), "set_maximum_cached_mbytes",
                  "get_maximum_cached_mbytes" );

    ClassDB::bind_method( D_METHOD( "get_loading_descendant_limit" ),
                          &Cesium3DTileset::get_loading_descendant_limit );
    ClassDB::bind_method( D_METHOD( "set_loading_descendant_limit", "p_loading_descendant_limit" ),
                          &Cesium3DTileset::set_loading_descendant_limit );
    ADD_PROPERTY( PropertyInfo( Variant::INT, "loading descendant limit" ),
                  "set_loading_descendant_limit", "get_loading_descendant_limit" );

    ClassDB::bind_method( D_METHOD( "get_enable_frustum_culling" ),
                          &Cesium3DTileset::get_enable_frustum_culling );
    ClassDB::bind_method( D_METHOD( "set_enable_frustum_culling", "p_enable_frustum_culling" ),
                          &Cesium3DTileset::set_enable_frustum_culling );
    ADD_PROPERTY( PropertyInfo( Variant::BOOL, "enable frustum culling" ),
                  "set_enable_frustum_culling", "get_enable_frustum_culling" );

    ClassDB::bind_method( D_METHOD( "get_enable_fog_culling" ),
                          &Cesium3DTileset::get_enable_fog_culling );
    ClassDB::bind_method( D_METHOD( "set_enable_fog_culling", "p_enable_fog_culling" ),
                          &Cesium3DTileset::set_enable_fog_culling );
    ADD_PROPERTY( PropertyInfo( Variant::BOOL, "enable fog culling" ), "set_enable_fog_culling",
                  "get_enable_fog_culling" );

    ClassDB::bind_method( D_METHOD( "get_enforce_culled_screen_space_error" ),
                          &Cesium3DTileset::get_enforce_culled_screen_space_error );
    ClassDB::bind_method(
        D_METHOD( "set_enforce_culled_screen_space_error", "p_enforce_culled_screen_space_error" ),
        &Cesium3DTileset::set_enforce_culled_screen_space_error );
    ADD_PROPERTY( PropertyInfo( Variant::BOOL, "enforce culled screen space error" ),
                  "set_enforce_culled_screen_space_error",
                  "get_enforce_culled_screen_space_error" );

    ClassDB::bind_method( D_METHOD( "get_culled_screen_space_error" ),
                          &Cesium3DTileset::get_culled_screen_space_error );
    ClassDB::bind_method(
        D_METHOD( "set_culled_screen_space_error", "p_culled_screen_space_error" ),
        &Cesium3DTileset::set_culled_screen_space_error );
    ADD_PROPERTY( PropertyInfo( Variant::FLOAT, "culled screen space error" ),
                  "set_culled_screen_space_error", "get_culled_screen_space_error" );

    ClassDB::bind_method( D_METHOD( "get_suspend_update" ), &Cesium3DTileset::get_suspend_update );
    ClassDB::bind_method( D_METHOD( "set_suspend_update", "p_suspend_update" ),
                          &Cesium3DTileset::set_suspend_update );
    ADD_PROPERTY( PropertyInfo( Variant::BOOL, "suspend update" ), "set_suspend_update",
                  "get_suspend_update" );

    ClassDB::bind_method( D_METHOD( "get_create_physics_meshes" ),
                          &Cesium3DTileset::get_create_physics_meshes );
    ClassDB::bind_method( D_METHOD( "set_create_physics_meshes", "p_create_physics_meshes" ),
                          &Cesium3DTileset::set_create_physics_meshes );
    ADD_PROPERTY( PropertyInfo( Variant::BOOL, "create physics meshes" ),
                  "set_create_physics_meshes", "get_create_physics_meshes" );

    ClassDB::bind_method( D_METHOD( "get_generate_smooth_normals" ),
                          &Cesium3DTileset::get_generate_smooth_normals );
    ClassDB::bind_method( D_METHOD( "set_generate_smooth_normals", "p_generate_smooth_normals" ),
                          &Cesium3DTileset::set_generate_smooth_normals );
    ADD_PROPERTY( PropertyInfo( Variant::BOOL, "generate smooth normals" ),
                  "set_generate_smooth_normals", "get_generate_smooth_normals" );

    ClassDB::bind_method( D_METHOD( "get_log_selection_stats" ),
                          &Cesium3DTileset::get_log_selection_stats );
    ClassDB::bind_method( D_METHOD( "set_log_selection_stats", "p_log_selection_stats" ),
                          &Cesium3DTileset::set_log_selection_stats );
    ADD_PROPERTY( PropertyInfo( Variant::BOOL, "log selection stats" ), "set_log_selection_stats",
                  "get_log_selection_stats" );

    ClassDB::bind_method( D_METHOD( "get_imagery_provider" ),
                          &Cesium3DTileset::get_imagery_provider );
    ClassDB::bind_method( D_METHOD( "set_imagery_provider", "p_imagery_provider" ),
                          &Cesium3DTileset::set_imagery_provider );
    ADD_PROPERTY( PropertyInfo( Variant::OBJECT, "imagery_provider", PROPERTY_HINT_RESOURCE_TYPE,
                                "GTileMapServiceRasterOverlay,GDebugColorizeTilesRasterOverlay" ),
                  "set_imagery_provider", "get_imagery_provider" );

    ClassDB::bind_method( D_METHOD( "load_tileset" ), &Cesium3DTileset::load_tileset );
    ClassDB::bind_method( D_METHOD( "focus_tileset" ), &Cesium3DTileset::focus_tileset );
    ClassDB::bind_method( D_METHOD( "destroy_tileset" ), &Cesium3DTileset::destroy_tileset );
    ClassDB::bind_method( D_METHOD( "update", "delta" ), &Cesium3DTileset::update );

    ADD_SIGNAL( MethodInfo( "on_tileset_loaded" ) );
}

Cesium3DTileset::Cesium3DTileset() :
    p_tileset( nullptr ), tileset_source( TilesetSource::FromUrl ), url( "" ),
    maximum_screen_space_error( 16.0f ), preload_ancestors( true ), preload_siblings( true ),
    forbid_holes( true ), maximum_simultaneous_tile_loads( 20 ), maximum_cached_mbytes( 512 ),
    loading_descendant_limit( 20 ), enable_frustum_culling( true ), enable_fog_culling( true ),
    enforce_culled_screen_space_error( true ), culled_screen_space_error( 64.0f ),
    suspend_update( false ), create_physics_meshes( true ), generate_smooth_normals( false ),
    log_selection_stats( false ), last_opaque_material_hash( 0 ), load_progress( 0.0f ),
    active_loading( false )
{
}

Cesium3DTileset::~Cesium3DTileset()
{
    this->destroy_tileset();
}

const CesiumGeoreference *Cesium3DTileset::resolve_georeference() const
{
    Node *parent = this->get_parent();
    CesiumGeoreference *georeference = Object::cast_to<CesiumGeoreference>( parent );
    if ( georeference )
    {
        return georeference;
    }
    return nullptr;
}

const CesiumGeospatial::LocalHorizontalCoordinateSystem *Cesium3DTileset::get_georeference_crs()
    const
{
    Node *parent = this->get_parent();

    CesiumGeoreference *georeference = Object::cast_to<CesiumGeoreference>( parent );
    if ( georeference )
    {
        return &( georeference->getCoordinateSystem() );
    }
    return nullptr;
}

void Cesium3DTileset::load_tileset()
{
    TilesetOptions options{};
    options.maximumScreenSpaceError = this->maximum_screen_space_error;
    options.preloadAncestors = this->preload_ancestors;
    options.preloadSiblings = this->preload_siblings;
    options.forbidHoles = this->forbid_holes;
    options.maximumSimultaneousTileLoads = this->maximum_simultaneous_tile_loads;
    options.maximumCachedBytes = this->maximum_cached_mbytes * 1024 * 1024;
    options.loadingDescendantLimit = this->loading_descendant_limit;
    options.enableFrustumCulling = this->enable_frustum_culling;
    options.enableFogCulling = this->enable_fog_culling;
    options.enforceCulledScreenSpaceError = this->enforce_culled_screen_space_error;
    options.culledScreenSpaceError = this->culled_screen_space_error;
    options.loadErrorCallback = []( const TilesetLoadFailureDetails &details ) {
        uint16_t statusCode = details.statusCode;
        std::string message = details.message;
        godot::StringName message_( message.c_str() );
        UtilityFunctions::printerr( "Error message: ", message_, " status code: ", statusCode );
    };
    options.mainThreadLoadingTimeLimit = 5.0;
    options.tileCacheUnloadTimeLimit = 5.0;

    TilesetContentOptions contentOptions{};
    contentOptions.generateMissingNormalsSmooth = this->generate_smooth_normals;

    CesiumGltf::SupportedGpuCompressedPixelFormats supportedFormats;
    supportedFormats.ETC2_RGBA = true;
    supportedFormats.ETC1_RGB = true;
    supportedFormats.BC1_RGB = true;
    supportedFormats.BC3_RGBA = true;
    supportedFormats.BC4_R = true;
    supportedFormats.BC5_RG = true;
    supportedFormats.BC7_RGBA = true;
    supportedFormats.ASTC_4x4_RGBA = true;
    supportedFormats.PVRTC1_4_RGB = true;
    supportedFormats.PVRTC1_4_RGBA = true;
    supportedFormats.ETC2_EAC_R11 = true;
    supportedFormats.ETC2_EAC_RG11 = true;

    contentOptions.ktx2TranscodeTargets =
        CesiumGltf::Ktx2TranscodeTargets( supportedFormats, false );
    contentOptions.applyTextureTransform = false;
    options.contentOptions = contentOptions;

    const CesiumGeoreference *georeference = resolve_georeference();
    if ( !georeference )
    {
        return;
    }
    CesiumGeospatial::Ellipsoid p_native_ellipsoid =
        georeference->get_ellipsoid()->get_native_ellipsoid();
    options.ellipsoid = p_native_ellipsoid;

    this->last_update_result = ViewUpdateResult();

    std::string url_( url.utf8().get_data() );
    switch ( tileset_source )
    {
        case TilesetSource::FromUrl:
            UtilityFunctions::print( "Load tileset from url: ", url );
            this->p_tileset =
                std::make_unique<Tileset>( createTilesetExternals( this ), url_, options );
            break;
        case TilesetSource::FromEllipsoid:
            UtilityFunctions::print( "Load tileset from ellipsoid" );
            this->p_tileset = Cesium3DTilesSelection::EllipsoidTilesetLoader::createTileset(
                createTilesetExternals( this ), options );
            break;
    }

    if ( this->imagery_provider.is_valid() )
    {
        Ref<ImageryProvider> overlay = Ref<ImageryProvider>( imagery_provider.ptr() );
        if ( overlay.is_valid() )
        {
            overlay->addToTileset( this->p_tileset.get() );
            this->update_overlay_material_keys();
        }
    }
}

void Cesium3DTileset::destroy_tileset()
{
    if ( !this->p_tileset )
    {
        return;
    }
    UtilityFunctions::print( "Destory tileset" );
    this->destroy_imagery_provider();
    this->p_tileset.reset();
}

namespace
{
    struct CalculateECEFCameraPosition
    {
        const CesiumGeospatial::Ellipsoid &ellipsoid;

        glm::dvec3 operator()( const CesiumGeometry::BoundingSphere &sphere )
        {
            const glm::dvec3 &center = sphere.getCenter();
            glm::dmat4 enuToEcef = glm::dmat4(
                CesiumGeospatial::GlobeTransforms::eastNorthUpToFixedFrame( center, ellipsoid ) );
            glm::dvec3 offset = sphere.getRadius() * glm::normalize( glm::dvec3( enuToEcef[0] ) +
                                                                     glm::dvec3( enuToEcef[1] ) +
                                                                     glm::dvec3( enuToEcef[2] ) );
            glm::dvec3 position = center + offset;
            return position;
        }

        glm::dvec3 operator()( const CesiumGeometry::OrientedBoundingBox &orientedBoundingBox )
        {
            const glm::dvec3 &center = orientedBoundingBox.getCenter();
            glm::dmat4 enuToEcef = glm::dmat4(
                CesiumGeospatial::GlobeTransforms::eastNorthUpToFixedFrame( center, ellipsoid ) );
            const glm::dmat3 &halfAxes = orientedBoundingBox.getHalfAxes();
            glm::dvec3 offset =
                glm::length( halfAxes[0] + halfAxes[1] + halfAxes[2] ) *
                glm::normalize( glm::dvec3( enuToEcef[0] ) + glm::dvec3( enuToEcef[1] ) +
                                glm::dvec3( enuToEcef[2] ) );
            glm::dvec3 position = center + offset;
            return position;
        }

        glm::dvec3 operator()( const CesiumGeospatial::BoundingRegion &boundingRegion )
        {
            return ( *this )( boundingRegion.getBoundingBox() );
        }

        glm::dvec3 operator()( const CesiumGeospatial::BoundingRegionWithLooseFittingHeights
                                   &boundingRegionWithLooseFittingHeights )
        {
            return ( *this )(
                boundingRegionWithLooseFittingHeights.getBoundingRegion().getBoundingBox() );
        }

        glm::dvec3 operator()( const CesiumGeospatial::S2CellBoundingVolume &s2 )
        {
            return ( *this )( s2.computeBoundingRegion() );
        }
    };
} // namespace

void Cesium3DTileset::focus_tileset()
{
    UtilityFunctions::print( "Focus tileset" );

    if ( !this->p_tileset || !this->p_tileset->getRootTile() )
    {
        return;
    }

    godot::Camera3D *camera = nullptr;
    if ( godot::Engine::get_singleton()->is_editor_hint() )
    {
        EditorInterface *editor_interface = EditorInterface::get_singleton();
        godot::SubViewport *editor_viewport = editor_interface->get_editor_viewport_3d( 0 );
        if ( !editor_viewport )
        {
            return;
        }
        camera = editor_viewport->get_camera_3d();
    }
    else
    {
        godot::Viewport *viewport = this->get_viewport();
        if ( !viewport )
        {
            return;
        }
        camera = viewport->get_camera_3d();
    }

    if ( camera == nullptr )
    {
        return;
    }

    const CesiumGeospatial::LocalHorizontalCoordinateSystem *georeference_crs =
        get_georeference_crs();
    const glm::dmat4 &ecefToGodotWorld = georeference_crs->getEcefToLocalTransformation();
    const BoundingVolume &boundingVolume = this->p_tileset->getRootTile()->getBoundingVolume();
    glm::dvec3 ecefCameraPosition = std::visit(
        CalculateECEFCameraPosition{ CesiumGeospatial::Ellipsoid::WGS84 }, boundingVolume );
    glm::dvec3 godotCameraPosition =
        glm::dvec3( ecefToGodotWorld * glm::dvec4( ecefCameraPosition, 1.0 ) );

    glm::dvec3 ecefCenter = Cesium3DTilesSelection::getBoundingVolumeCenter( boundingVolume );
    glm::dvec3 godotCenter = glm::dvec3( ecefToGodotWorld * glm::dvec4( ecefCenter, 1.0 ) );
    glm::dvec3 godotCameraFront = glm::normalize( godotCenter - godotCameraPosition );
    glm::dvec3 godotCameraRight =
        glm::normalize( glm::cross( glm::dvec3( 0.0, 0.0, 1.0 ), godotCameraFront ) );
    glm::dvec3 godotCameraUp = glm::normalize( glm::cross( godotCameraFront, godotCameraRight ) );

    godot::Vector3 cameraPosition( godotCameraPosition.x, godotCameraPosition.y,
                                   godotCameraPosition.z );
    godot::Vector3 cameraFront( godotCameraFront.x, godotCameraFront.y, godotCameraFront.z );
    godot::Vector3 cameraRight( godotCameraRight.x, godotCameraRight.y, godotCameraRight.z );
    godot::Vector3 CameraUp( godotCameraUp.x, godotCameraUp.y, godotCameraUp.z );

    camera->set_transform(
        godot::Transform3D( cameraRight, CameraUp, cameraFront, cameraPosition ) );
}

Cesium3DTilesSelection::Tileset *Cesium3DTileset::get_tileset()
{
    return this->p_tileset.get();
}

const Cesium3DTilesSelection::Tileset *Cesium3DTileset::get_tileset() const
{
    return this->p_tileset.get();
}

float Cesium3DTileset::compute_load_progress()
{
    if ( !get_tileset() )
    {
        return 0;
    }
    return get_tileset()->computeLoadProgress();
}

void Cesium3DTileset::update_load_status()
{
    this->load_progress = this->p_tileset->computeLoadProgress();
    if ( this->load_progress < 100 || this->last_update_result.tilesWaitingForOcclusionResults > 0 )
    {
        this->active_loading = true;
    }
    else if ( this->active_loading && this->load_progress == 100 )
    {
        // There might be a few frames where nothing needs to be loaded as we
        // are waiting for occlusion results to come back, which means we are not
        // done with loading all the tiles in the tileset yet.
        if ( this->last_update_result.tilesWaitingForOcclusionResults == 0 )
        {
            // Tileset just finished loading, we broadcast the update
            UtilityFunctions::print( "Broadcasting OnTileLoaded" );
            // Tileset remains 100% loaded if we don't have to reload it
            // so we don't want to keep on sending finished loading updates
            this->active_loading = false;
        }
    }
}

void Cesium3DTileset::update_tileset_options_from_properties()
{
    Cesium3DTilesSelection::TilesetOptions &options = this->p_tileset->getOptions();
    options.maximumScreenSpaceError = this->maximum_screen_space_error;
    options.preloadAncestors = this->preload_ancestors;
    options.preloadSiblings = this->preload_siblings;
    options.forbidHoles = this->forbid_holes;
    options.maximumSimultaneousTileLoads = this->maximum_simultaneous_tile_loads;
    options.maximumCachedBytes = this->maximum_cached_mbytes * 1024 * 1024;
    options.loadingDescendantLimit = this->loading_descendant_limit;
    options.enableFrustumCulling = this->enable_frustum_culling;
    options.enableFogCulling = this->enable_fog_culling;
    options.enforceCulledScreenSpaceError = this->enforce_culled_screen_space_error;
    options.culledScreenSpaceError = this->culled_screen_space_error;
}

void Cesium3DTileset::update_last_view_update_result_state(
    const Cesium3DTilesSelection::ViewUpdateResult &result )
{
    if ( !this->log_selection_stats )
    {
        return;
    }
    const ViewUpdateResult &previousResult = this->last_update_result;
    if ( result.tilesToRenderThisFrame.size() != previousResult.tilesToRenderThisFrame.size() ||
         result.workerThreadTileLoadQueueLength != previousResult.workerThreadTileLoadQueueLength ||
         result.mainThreadTileLoadQueueLength != previousResult.mainThreadTileLoadQueueLength ||
         result.tilesVisited != previousResult.tilesVisited ||
         result.culledTilesVisited != previousResult.culledTilesVisited ||
         result.tilesCulled != previousResult.tilesCulled ||
         result.tilesOccluded != previousResult.tilesOccluded ||
         result.tilesWaitingForOcclusionResults != previousResult.tilesWaitingForOcclusionResults ||
         result.maxDepthVisited != previousResult.maxDepthVisited )
    {
        this->last_update_result = result;
        godot::String name = this->get_name();
        std::string tilesetName = name.utf8().get_data();

        SPDLOG_LOGGER_INFO(
            this->p_tileset->getExternals().pLogger,
            "{0}: Visited {1}, Culled Visited {2}, Rendered {3}, Culled {4}, Occluded {5}, "
            "Waiting For Occlusion Results {6}, Max Depth Visited {7}, Loading-Worker {8}, "
            "Loading-Main {9} "
            "Total Tiles Resident {10}, Loaded tiles {11}%, Frame {12}",
            tilesetName, result.tilesVisited, result.culledTilesVisited,
            result.tilesToRenderThisFrame.size(), result.tilesCulled, result.tilesOccluded,
            result.tilesWaitingForOcclusionResults, result.maxDepthVisited,
            result.workerThreadTileLoadQueueLength, result.mainThreadTileLoadQueueLength,
            this->p_tileset->getNumberOfTilesLoaded(), this->load_progress, result.frameNumber );
    }
}

void Cesium3DTileset::update_overlay_material_keys()
{
    if ( !this->p_tileset || !this->p_tileset->getExternals().pPrepareRendererResources )
    {
        return;
    }

    std::vector<std::string> overlay_material_keys{
        imagery_provider->get_material_key().utf8().get_data()
    };
    GodotPrepareRendererResources *pRendererResources =
        static_cast<GodotPrepareRendererResources *>(
            p_tileset->getExternals().pPrepareRendererResources.get() );
    pRendererResources->getMaterialProperties().updateOverlayParameterIDs( overlay_material_keys );
}

void Cesium3DTileset::_process( double delta )
{
    this->update( delta );
}

void Cesium3DTileset::update( double delta )
{

    if ( this->get_suspend_update() )
    {
        return;
    }
    if ( !this->p_tileset )
    {
        this->load_tileset();
        if ( !this->p_tileset )
        {
            return;
        }
    }

    this->update_tileset_options_from_properties();

    std::vector<ViewState> viewStates = CameraManager::getAllCameras( *this );
    const ViewUpdateResult &updateResult =
        this->p_tileset->updateView( viewStates, static_cast<float>( delta ) );

    this->update_last_view_update_result_state( updateResult );

    for ( auto pTile : updateResult.tilesFadingOut )
    {
        if ( pTile->getState() != TileLoadState::Done )
        {
            continue;
        }

        const Cesium3DTilesSelection::TileContent &content = pTile->getContent();
        const Cesium3DTilesSelection::TileRenderContent *pRenderContent =
            content.getRenderContent();
        if ( pRenderContent )
        {
            CesiumGltfNode *pCesiumGltfNode =
                static_cast<CesiumGltfNode *>( pRenderContent->getRenderResources() );
            if ( pCesiumGltfNode )
            {
                pCesiumGltfNode->set_visible( false );
            }
        }
    }

    for ( auto pTile : updateResult.tilesToRenderThisFrame )
    {
        if ( pTile->getState() != TileLoadState::Done )
        {
            continue;
        }
        const Cesium3DTilesSelection::TileContent &content = pTile->getContent();
        const Cesium3DTilesSelection::TileRenderContent *pRenderContent =
            content.getRenderContent();
        if ( pRenderContent )
        {
            CesiumGltfNode *pCesiumGltfNode =
                static_cast<CesiumGltfNode *>( pRenderContent->getRenderResources() );
            if ( pCesiumGltfNode )
            {
                pCesiumGltfNode->set_visible( true );
            }
        }
    }

    this->update_load_status();
}

void Cesium3DTileset::set_tileset_source( const TilesetSource p_tileset_source )
{
    if ( tileset_source != p_tileset_source )
    {
        tileset_source = p_tileset_source;
        if ( tileset_source == TilesetSource::FromEllipsoid )
        {
            // TODO: disable url property and enable imagery_provider property
        }
    }
}

TilesetSource Cesium3DTileset::get_tileset_source()
{
    return tileset_source;
}

TypedArray<ImageryProvider> Cesium3DTileset::get_test_array()
{
    return test_array;
}

void Cesium3DTileset::set_test_array( TypedArray<ImageryProvider> p_test_array )
{
    test_array = p_test_array;
}

void Cesium3DTileset::set_url( const String p_url )
{
    if ( url != p_url )
    {
        url = p_url;
        UtilityFunctions::print( "set_url->", url );
        this->destroy_tileset();
    }
}
String Cesium3DTileset::get_url() const
{
    return url;
}

float Cesium3DTileset::get_maximum_screen_space_error() const
{
    return this->maximum_screen_space_error;
}
void Cesium3DTileset::set_maximum_screen_space_error( const float p_maximum_screen_space_error )
{
    if ( maximum_screen_space_error != p_maximum_screen_space_error )
    {
        this->maximum_screen_space_error = p_maximum_screen_space_error;
    }
}

bool Cesium3DTileset::get_preload_ancestors() const
{
    return this->preload_ancestors;
}
void Cesium3DTileset::set_preload_ancestors( const bool p_preload_ancestors )
{
    if ( this->preload_ancestors != p_preload_ancestors )
    {
        this->preload_ancestors = p_preload_ancestors;
    }
}

bool Cesium3DTileset::get_preload_siblings() const
{
    return this->preload_siblings;
}
void Cesium3DTileset::set_preload_siblings( const bool p_preload_siblings )
{
    if ( this->preload_siblings != p_preload_siblings )
    {
        this->preload_siblings = p_preload_siblings;
    }
}

bool Cesium3DTileset::get_forbid_holes() const
{
    return this->forbid_holes;
}
void Cesium3DTileset::set_forbid_holes( const bool p_forbid_holes )
{
    if ( this->forbid_holes != p_forbid_holes )
    {
        this->forbid_holes = p_forbid_holes;
    }
}

unsigned int Cesium3DTileset::get_maximum_simultaneous_tile_loads() const
{
    return this->maximum_simultaneous_tile_loads;
}
void Cesium3DTileset::set_maximum_simultaneous_tile_loads(
    const unsigned int p_maximum_simultaneous_tile_loads )
{
    if ( this->maximum_simultaneous_tile_loads != p_maximum_simultaneous_tile_loads )
    {
        this->maximum_simultaneous_tile_loads = p_maximum_simultaneous_tile_loads;
    }
}

unsigned int Cesium3DTileset::get_maximum_cached_mbytes() const
{
    return this->maximum_cached_mbytes;
}
void Cesium3DTileset::set_maximum_cached_mbytes( const unsigned int p_maximum_cached_mbytes )
{
    if ( this->maximum_cached_mbytes != p_maximum_cached_mbytes )
    {
        this->maximum_cached_mbytes = p_maximum_cached_mbytes;
    }
}

unsigned int Cesium3DTileset::get_loading_descendant_limit() const
{
    return this->loading_descendant_limit;
}
void Cesium3DTileset::set_loading_descendant_limit( const unsigned int p_loading_descendant_limit )
{
    if ( this->loading_descendant_limit != p_loading_descendant_limit )
    {
        this->loading_descendant_limit = p_loading_descendant_limit;
    }
}

bool Cesium3DTileset::get_enable_frustum_culling() const
{
    return this->enable_frustum_culling;
}
void Cesium3DTileset::set_enable_frustum_culling( const bool p_enable_frustum_culling )
{
    if ( this->enable_frustum_culling != p_enable_frustum_culling )
    {
        this->enable_frustum_culling = p_enable_frustum_culling;
    }
}

bool Cesium3DTileset::get_enable_fog_culling() const
{
    return this->enable_fog_culling;
}
void Cesium3DTileset::set_enable_fog_culling( const bool p_enable_fog_culling )
{
    if ( this->enable_fog_culling != p_enable_fog_culling )
    {
        this->enable_fog_culling = p_enable_fog_culling;
    }
}

bool Cesium3DTileset::get_enforce_culled_screen_space_error() const
{
    return this->enforce_culled_screen_space_error;
}
void Cesium3DTileset::set_enforce_culled_screen_space_error(
    const bool p_enforce_culled_screen_space_error )
{
    if ( this->enforce_culled_screen_space_error != p_enforce_culled_screen_space_error )
    {
        this->enforce_culled_screen_space_error = p_enforce_culled_screen_space_error;
    }
}

float Cesium3DTileset::get_culled_screen_space_error() const
{
    return this->culled_screen_space_error;
}
void Cesium3DTileset::set_culled_screen_space_error( const float p_culled_screen_space_error )
{
    if ( this->culled_screen_space_error != p_culled_screen_space_error )
    {
        this->culled_screen_space_error = p_culled_screen_space_error;
    }
}

bool Cesium3DTileset::get_suspend_update() const
{
    return this->suspend_update;
}
void Cesium3DTileset::set_suspend_update( const bool p_suspend_update )
{
    if ( this->suspend_update != p_suspend_update )
    {
        this->suspend_update = p_suspend_update;
    }
}

bool Cesium3DTileset::get_create_physics_meshes() const
{
    return this->create_physics_meshes;
}
void Cesium3DTileset::set_create_physics_meshes( const bool p_create_physics_meshes )
{
    if ( this->create_physics_meshes != p_create_physics_meshes )
    {
        this->create_physics_meshes = p_create_physics_meshes;
        this->destroy_tileset();
    }
}

bool Cesium3DTileset::get_generate_smooth_normals() const
{
    return this->generate_smooth_normals;
}
void Cesium3DTileset::set_generate_smooth_normals( const bool p_generate_smooth_normals )
{
    if ( this->generate_smooth_normals != p_generate_smooth_normals )
    {
        this->generate_smooth_normals = p_generate_smooth_normals;
        this->destroy_tileset();
    }
}

void Cesium3DTileset::set_log_selection_stats( const bool p_log_selection_stats )
{
    this->log_selection_stats = p_log_selection_stats;
}

bool Cesium3DTileset::get_log_selection_stats() const
{
    return this->log_selection_stats;
}

Ref<ImageryProvider> Cesium3DTileset::get_imagery_provider() const
{
    return this->imagery_provider;
}

void Cesium3DTileset::set_imagery_provider( const Ref<ImageryProvider> p_imagery_provider )
{
    if ( this->imagery_provider != p_imagery_provider )
    {
        this->imagery_provider = p_imagery_provider;
        this->destroy_tileset();
    }
}

void Cesium3DTileset::destroy_imagery_provider()
{
    if ( this->imagery_provider.is_valid() && this->p_tileset )
    {
        imagery_provider->removeFromTileset( this->p_tileset.get() );
    }
}