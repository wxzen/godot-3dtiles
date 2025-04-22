#ifndef GODOT_PREPARE_RENDERER_RESOURCES_H
#define GODOT_PREPARE_RENDERER_RESOURCES_H

#include "Cesium3DTileset.h"
#include <Cesium3DTilesSelection/IPrepareRendererResources.h>
#include <Cesium3DTilesSelection/Tile.h>
#include <Cesium3DTilesSelection/Tileset.h>
#include <CesiumGeometry/Transforms.h>
#include <CesiumGltf/AccessorView.h>
#include <CesiumGltf/ExtensionExtMeshFeatures.h>
#include <CesiumGltf/ExtensionKhrMaterialsUnlit.h>
#include <CesiumGltf/ExtensionKhrTextureTransform.h>
#include <CesiumGltf/ExtensionModelExtStructuralMetadata.h>
#include <CesiumGltf/KhrTextureTransform.h>
#include <CesiumGltfContent/GltfUtilities.h>
#include <CesiumGltfReader/GltfReader.h>
#include <CesiumUtility/ScopeGuard.h>
#include <spdlog/spdlog.h>

#include "godot_cpp/classes/node.hpp"
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/quaternion.hpp>
#include <godot_cpp/classes/array_mesh.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/image_texture.hpp>
#include <godot_cpp/classes/mesh_instance3d.hpp>
#include <godot_cpp/classes/random_number_generator.hpp>
#include <godot_cpp/templates/vector.hpp>
#include <godot_cpp/variant/color.hpp>

using namespace godot;

namespace CesiumForGodot
{
    enum class IndexFormat
    {
        UInt16 = 0,
        UInt32 = 1,
    };

    enum class VertexAttribute
    {
        Position = 0,
        Normal = 1,
        Tangent = 2,
        Color = 3,
        TexCoord0 = 4,
        TexCoord1 = 5,
        BlendWeight = 12,
        BlendIndices = 13,
    };

    enum class VertexAttributeFormat
    {
        Float32 = 0,
        Float16 = 1,
        UNorm8 = 2,
        SNorm8 = 3,
        UNorm16 = 4,
        SNorm16 = 5,
        UInt8 = 6,
        SInt8 = 7,
        UInt16 = 8,
        SInt16 = 9,
        UInt32 = 10,
        SInt32 = 11,
    };

    struct VertexAttributeDescriptor
    {
        VertexAttribute attribute;
        VertexAttributeFormat format;
        std::int32_t dimension;
    };

    /**
     * @brief Information about how a given glTF primitive was converted into
     * Godot MeshData.
     */
    struct CesiumPrimitiveInfo
    {
        /**
         * @brief Whether or not the primitive's mode is set to POINTS.
         * This affects whether or not it can be baked into a physics mesh.
         */
        bool containsPoints = false;

        /**
         * @brief Whether or not the primitive contains translucent vertex
         * colors. This can affect material tags used to render the model.
         */
        bool isTranslucent = false;

        /**
         * @brief Whether or not the primitive material has the KHR_materials_unlit
         * extension.
         */
        bool isUnlit = false;

        /**
         * @brief Maps a texture coordinate index i (TEXCOORD_<i>) to the
         * corresponding Godot texture coordinate index.
         */
        std::unordered_map<uint32_t, uint32_t> uvIndexMap{};

        /**
         * @brief Maps an overlay texture coordinate index i (_CESIUMOVERLAY_<i>) to
         * the corresponding Godot texture coordinate index.
         */
        std::unordered_map<uint32_t, uint32_t> rasterOverlayUvIndexMap{};
    };

    /**
     * @brief The fully loaded Node3D object for this glTF and associated information.
     */
    struct CesiumGltfNode
    {
        /**
         * @brief The fully loaded Godot Node3D object for this glTF.
         */
        std::vector<MeshInstance3D *> pNodes;

        /**
         * @brief Information about how glTF mesh primitives were translated to Godot
         * meshes.
         */
        std::vector<CesiumPrimitiveInfo> primitiveInfos{};

        void set_visible( bool b )
        {
            for ( MeshInstance3D *inst : pNodes )
            {
                inst->set_visible( b );
            }
            visible = b;
        }

        bool visible = false;
        bool isFreed = false;
    };

    class GodotPrepareRendererResources : public Cesium3DTilesSelection::IPrepareRendererResources
    {
    public:
        GodotPrepareRendererResources( Cesium3DTileset *tileset );

        virtual CesiumAsync::Future<Cesium3DTilesSelection::TileLoadResultAndRenderResources>
            prepareInLoadThread( const CesiumAsync::AsyncSystem &asyncSystem,
                                 Cesium3DTilesSelection::TileLoadResult &&tileLoadResult,
                                 const glm::dmat4 &transform,
                                 const std::any &rendererOptions ) override;

        virtual void *prepareInMainThread( Cesium3DTilesSelection::Tile &tile,
                                           void *pLoadThreadResult ) override;

        virtual void free( Cesium3DTilesSelection::Tile &tile, void *pLoadThreadResult,
                           void *pMainThreadResult ) noexcept override;

        virtual void *prepareRasterInLoadThread( CesiumGltf::ImageAsset &image,
                                                 const std::any &rendererOptions ) override;

        virtual void *prepareRasterInMainThread(
            CesiumRasterOverlays::RasterOverlayTile &rasterTile, void *pLoadThreadResult ) override;

        virtual void freeRaster( const CesiumRasterOverlays::RasterOverlayTile &rasterTile,
                                 void *pLoadThreadResult,
                                 void *pMainThreadResult ) noexcept override;

        virtual void attachRasterInMainThread(
            const Cesium3DTilesSelection::Tile &tile, int32_t overlayTextureCoordinateID,
            const CesiumRasterOverlays::RasterOverlayTile &rasterTile,
            void *pMainThreadRendererResources, const glm::dvec2 &translation,
            const glm::dvec2 &scale ) override;

        virtual void detachRasterInMainThread(
            const Cesium3DTilesSelection::Tile &tile, int32_t overlayTextureCoordinateID,
            const CesiumRasterOverlays::RasterOverlayTile &rasterTile,
            void *pMainThreadRendererResources ) noexcept override;

    private:
        Cesium3DTileset *_tileset;
    };

    template <typename TIndex>
    void computeFlatNormals( uint8_t *pWritePos, size_t stride, TIndex *indices, int32_t indexCount,
                             const CesiumGltf::AccessorView<glm::vec3> &positionView )
    {

        for ( int i = 0; i < indexCount; i += 3 )
        {
            TIndex i0 = indices[i];
            TIndex i1 = indices[i + 1];
            TIndex i2 = indices[i + 2];

            const glm::vec3 &v0 = *reinterpret_cast<const glm::vec3 *>( &positionView[i0] );
            const glm::vec3 &v1 = *reinterpret_cast<const glm::vec3 *>( &positionView[i1] );
            const glm::vec3 &v2 = *reinterpret_cast<const glm::vec3 *>( &positionView[i2] );

            glm::vec3 normal = glm::normalize( glm::cross( v1 - v0, v2 - v0 ) );
            for ( int j = 0; j < 3; j++ )
            {
                *reinterpret_cast<glm::vec3 *>( pWritePos ) = normal;
                pWritePos += stride;
            }
        }
    }

} // namespace CesiumForGodot

#endif