#ifndef CESIUM_3DTILESET_H
#define CESIUM_3DTILESET_H

#include <godot_cpp/classes/mesh_instance3d.hpp>
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/core/binder_common.hpp>
#include <godot_cpp/variant/typed_array.hpp>

#include <Cesium3DTilesSelection/Tileset.h>
#include <Cesium3DTilesSelection/TilesetExternals.h>
#include <Cesium3DTilesSelection/ViewUpdateResult.h>
#include <CesiumGeospatial/LocalHorizontalCoordinateSystem.h>

#include "CesiumGeoreference.h"
#include "CesiumImageryProvider.h"

using namespace godot;

namespace CesiumForGodot
{
    enum TilesetSource
    {
        /**
         * The tileset will be loaded from the specified Url.
         */
        FromUrl,

        /**
         * The tileset will be loaded from the georeference ellipsoid.
         */
        FromEllipsoid
    };

    /**
     * @class Cesium3DTileset
     * @brief 3D Tileset loader and renderer for Cesium tilesets.
     *
     * This class is responsible for loading and rendering 3D tilesets in Cesium format.
     */
    class Cesium3DTileset : public Node3D
    {
        GDCLASS( Cesium3DTileset, Node3D )

    private:
        std::unique_ptr<Cesium3DTilesSelection::Tileset> p_tileset;
        Cesium3DTilesSelection::ViewUpdateResult last_update_result;

        TilesetSource tileset_source;
        String url;
        float maximum_screen_space_error;
        bool preload_ancestors;
        bool preload_siblings;
        bool forbid_holes = false;
        unsigned int maximum_simultaneous_tile_loads;
        unsigned int maximum_cached_mbytes;
        unsigned int loading_descendant_limit;
        bool enable_frustum_culling;
        bool enable_fog_culling;
        bool enforce_culled_screen_space_error;
        float culled_screen_space_error;
        bool suspend_update;
        bool create_physics_meshes;
        bool generate_smooth_normals;

        /* Whether to log details about the tile selection process. */
        bool log_selection_stats;
        int32_t last_opaque_material_hash;
        float load_progress;
        bool active_loading;

        Ref<ImageryProvider> imagery_provider;

        TypedArray<ImageryProvider> test_array;

        void destroy_tileset();
        void load_tileset();
        void update_last_view_update_result_state(
            const Cesium3DTilesSelection::ViewUpdateResult &currentResult );
        float compute_load_progress();
        void update_load_status();
        void update_tileset_options_from_properties();
        void update_overlay_material_keys();

    protected:
        static void _bind_methods();

    public:
        Cesium3DTileset();

        virtual ~Cesium3DTileset();

        void _process( double delta ) override;
        void update( double delta );

        const CesiumGeoreference *resolve_georeference() const;
        const CesiumGeospatial::LocalHorizontalCoordinateSystem *get_georeference_crs() const;
        void focus_tileset();
        Cesium3DTilesSelection::Tileset *get_tileset();
        const Cesium3DTilesSelection::Tileset *get_tileset() const;

        void set_tileset_source( const TilesetSource );
        TilesetSource get_tileset_source();

        TypedArray<ImageryProvider> get_test_array();
        void set_test_array( TypedArray<ImageryProvider> p_test_array );

        void set_url( const String p_url );
        String get_url() const;
        float get_maximum_screen_space_error() const;
        void set_maximum_screen_space_error( const float p_maximum_screen_space_error );
        bool get_preload_ancestors() const;
        void set_preload_ancestors( const bool p_preload_ancestors );
        bool get_preload_siblings() const;
        void set_preload_siblings( const bool p_preload_siblings );
        bool get_forbid_holes() const;
        void set_forbid_holes( const bool p_forbid_holes );
        unsigned int get_maximum_simultaneous_tile_loads() const;
        void set_maximum_simultaneous_tile_loads(
            const unsigned int p_maximum_simultaneous_tile_loads );
        unsigned int get_maximum_cached_mbytes() const;
        void set_maximum_cached_mbytes( const unsigned int p_maximum_cached_mbytes );
        unsigned int get_loading_descendant_limit() const;
        void set_loading_descendant_limit( const unsigned int p_loading_descendant_limit );
        bool get_enable_frustum_culling() const;
        void set_enable_frustum_culling( const bool p_enable_frustum_culling );
        bool get_enable_fog_culling() const;
        void set_enable_fog_culling( const bool p_enable_fog_culling );
        bool get_enforce_culled_screen_space_error() const;
        void set_enforce_culled_screen_space_error(
            const bool p_enforce_culled_screen_space_error );
        float get_culled_screen_space_error() const;
        void set_culled_screen_space_error( const float p_culled_screen_space_error );
        bool get_suspend_update() const;
        void set_suspend_update( const bool p_suspend_update );
        bool get_create_physics_meshes() const;
        void set_create_physics_meshes( const bool p_create_physics_meshes );
        bool get_generate_smooth_normals() const;
        void set_generate_smooth_normals( const bool p_generate_smooth_normals );
        void set_log_selection_stats( const bool p_log_selection_stats );
        bool get_log_selection_stats() const;

        Ref<ImageryProvider> get_imagery_provider() const;
        void set_imagery_provider( const Ref<ImageryProvider> p_imagery_provider );
        void destroy_imagery_provider();
    };

} // namespace CesiumForGodot

VARIANT_ENUM_CAST( CesiumForGodot::TilesetSource );

#endif