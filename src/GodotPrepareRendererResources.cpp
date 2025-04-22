#include "GodotPrepareRendererResources.h"
#include <CesiumGltf/AccessorView.h>
#include <algorithm>

#include <godot_cpp/classes/standard_material3d.hpp>

using namespace CesiumForGodot;
using namespace CesiumRasterOverlays;
using namespace CesiumGltfContent;
using namespace Cesium3DTilesSelection;
using namespace CesiumGeometry;
using namespace CesiumGeospatial;
using namespace CesiumGltf;
using namespace CesiumUtility;

struct LoadThreadResult
{
    std::vector<Ref<ArrayMesh>> meshes;
    std::vector<CesiumPrimitiveInfo> primitiveInfos{};
};

bool isDegenerateTriangleMesh( const Ref<ArrayMesh> mesh )
{
    Array surface_array = mesh->surface_get_arrays( 0 );
    PackedVector3Array vertices = surface_array[ArrayMesh::ARRAY_VERTEX];
    int32_t vertexCount = vertices.size();
    if ( vertexCount < 3 )
    {
        return true;
    }

    if ( vertexCount == 3 )
    {
        godot::Vector3 Vertex0 = vertices[0];
        godot::Vector3 Vertex1 = vertices[1];
        godot::Vector3 Vertex2 = vertices[2];

        const real_t EPSILON = 1e-5f;
        return ( Vertex0.distance_to( Vertex1 ) < EPSILON ) ||
               ( Vertex1.distance_to( Vertex2 ) < EPSILON ) ||
               ( Vertex2.distance_to( Vertex0 ) < EPSILON );
    }

    return false;
}

int32_t countPrimitives( const CesiumGltf::Model &model )
{
    int32_t numberOfPrimitives = 0;
    model.forEachPrimitiveInScene(
        -1, [&numberOfPrimitives]( const CesiumGltf::Model &gltf, const CesiumGltf::Node &node,
                                   const CesiumGltf::Mesh &mesh,
                                   const CesiumGltf::MeshPrimitive &primitive,
                                   const glm::dmat4 &transform ) { ++numberOfPrimitives; } );
    return numberOfPrimitives;
}

void generateMipMaps( CesiumGltf::Model *pModel,
                      const std::optional<CesiumGltf::TextureInfo> &textureInfo )
{
    if ( textureInfo )
    {
        CesiumGltf::Texture *pTexture =
            CesiumGltf::Model::getSafe( &pModel->textures, textureInfo->index );
        if ( pTexture )
        {
            CesiumGltf::Image *pImage =
                CesiumGltf::Model::getSafe( &pModel->images, pTexture->source );
            const Sampler *pSampler =
                CesiumGltf::Model::getSafe( &pModel->samplers, pTexture->sampler );
            if ( pImage && pSampler )
            {
                switch ( pSampler->minFilter.value_or(
                    CesiumGltf::Sampler::MinFilter::LINEAR_MIPMAP_LINEAR ) )
                {
                    case CesiumGltf::Sampler::MinFilter::LINEAR_MIPMAP_LINEAR:
                    case CesiumGltf::Sampler::MinFilter::LINEAR_MIPMAP_NEAREST:
                    case CesiumGltf::Sampler::MinFilter::NEAREST_MIPMAP_LINEAR:
                    case CesiumGltf::Sampler::MinFilter::NEAREST_MIPMAP_NEAREST:
                        CesiumGltfReader::ImageDecoder::generateMipMaps( *pImage->pAsset );
                }
            }
        }
    }
}

void generateMipMapsForPrimitive( CesiumGltf::Model *pModel,
                                  const CesiumGltf::MeshPrimitive &primitive )
{
    const CesiumGltf::Material *pMaterial =
        CesiumGltf::Model::getSafe( &pModel->materials, primitive.material );
    if ( pMaterial )
    {
        if ( pMaterial->pbrMetallicRoughness )
        {
            generateMipMaps( pModel, pMaterial->pbrMetallicRoughness->baseColorTexture );
            generateMipMaps( pModel, pMaterial->pbrMetallicRoughness->metallicRoughnessTexture );
        }
        generateMipMaps( pModel, pMaterial->normalTexture );
        generateMipMaps( pModel, pMaterial->occlusionTexture );
        generateMipMaps( pModel, pMaterial->emissiveTexture );
    }
}

template <typename TIndex> std::vector<TIndex> generateIndices( const int32_t count )
{
    std::vector<TIndex> syntheticIndexBuffer( count );
    for ( int64_t i = 0; i < count; ++i )
    {
        syntheticIndexBuffer[i] = static_cast<TIndex>( i );
    }
    return syntheticIndexBuffer;
}

bool validateVertexColors( const CesiumGltf::Model &model, uint32_t accessorId, size_t vertexCount )
{
    if ( accessorId >= model.accessors.size() )
    {
        return false;
    }

    const CesiumGltf::Accessor &colorAccessor = model.accessors[accessorId];
    if ( colorAccessor.type != CesiumGltf::Accessor::Type::VEC3 &&
         colorAccessor.type != CesiumGltf::Accessor::Type::VEC4 )
    {
        return false;
    }

    if ( colorAccessor.componentType != CesiumGltf::Accessor::ComponentType::UNSIGNED_BYTE &&
         colorAccessor.componentType != CesiumGltf::Accessor::ComponentType::UNSIGNED_SHORT &&
         colorAccessor.componentType != CesiumGltf::Accessor::ComponentType::FLOAT )
    {
        return false;
    }

    if ( static_cast<size_t>( colorAccessor.count ) < vertexCount )
    {
        return false;
    }

    return true;
}
template <typename TIndex> struct CopyVertexColors
{
    uint8_t *pWritePos;
    size_t stride;
    size_t vertexCount;
    bool duplicateVertices;
    TIndex *indices;

    struct Color32
    {
        uint8_t r;
        uint8_t g;
        uint8_t b;
        uint8_t a;
    };

    bool operator()( AccessorView<nullptr_t> &&invalidView )
    {
        return false;
    }

    template <typename TColorView> bool operator()( TColorView &&colorView )
    {
        if ( colorView.status() != AccessorViewStatus::Valid )
        {
            return false;
        }

        bool success = true;
        if ( duplicateVertices )
        {
            for ( size_t i = 0; success && i < vertexCount; ++i )
            {
                TIndex vertexIndex = indices[i];
                if ( vertexIndex < 0 || vertexIndex >= colorView.size() )
                {
                    success = false;
                }
                else
                {
                    Color32 &packedColor = *reinterpret_cast<Color32 *>( pWritePos );
                    success = CopyVertexColors::convertColor( colorView[vertexIndex], packedColor );
                    pWritePos += stride;
                }
            }
        }
        else
        {
            for ( size_t i = 0; success && i < vertexCount; ++i )
            {
                if ( i >= static_cast<size_t>( colorView.size() ) )
                {
                    success = false;
                }
                else
                {
                    Color32 &packedColor = *reinterpret_cast<Color32 *>( pWritePos );
                    success = CopyVertexColors::convertColor( colorView[i], packedColor );
                    pWritePos += stride;
                }
            }
        }

        return success;
    }

    bool packColorChannel( uint8_t c, uint8_t &result )
    {
        result = c;
        return true;
    }

    bool packColorChannel( uint16_t c, uint8_t &result )
    {
        result = static_cast<uint8_t>( c >> 8 );
        return true;
    }

    bool packColorChannel( float c, uint8_t &result )
    {
        result = static_cast<uint8_t>( static_cast<uint32_t>( 255.0f * c ) & 255 );
        return true;
    }

    template <typename T> bool packColorChannel( T c, uint8_t &result )
    {
        // Invalid accessor type.
        return false;
    }

    template <typename TChannel>
    bool convertColor( const AccessorTypes::VEC3<TChannel> &color, Color32 &result )
    {
        result.a = 255;
        return packColorChannel( color.value[0], result.r ) &&
               packColorChannel( color.value[1], result.g ) &&
               packColorChannel( color.value[2], result.b );
    }

    template <typename TChannel>
    bool convertColor( const AccessorTypes::VEC4<TChannel> &color, Color32 &result )
    {
        return packColorChannel( color.value[0], result.r ) &&
               packColorChannel( color.value[1], result.g ) &&
               packColorChannel( color.value[2], result.b ) &&
               packColorChannel( color.value[3], result.a );
    }

    template <typename T> bool convertColor( T color, Color32 &result )
    {
        // Not an accessor
        return false;
    }
};

int32_t computeVertexDataSize( int32_t vertexCount, VertexAttributeDescriptor *attributes,
                               std::int32_t size )
{
    int32_t totalSize = 0;
    for ( int32_t i = 0; i < size; ++i )
    {
        const auto &attribute = attributes[i];
        size_t attributeSize = 0;
        switch ( attribute.format )
        {
            case VertexAttributeFormat::Float32:
            case VertexAttributeFormat::UInt32:
            case VertexAttributeFormat::SInt32:
                attributeSize = sizeof( float ) * attribute.dimension;
                break;
            case VertexAttributeFormat::Float16:
            case VertexAttributeFormat::UNorm16:
            case VertexAttributeFormat::SNorm16:
            case VertexAttributeFormat::UInt16:
            case VertexAttributeFormat::SInt16:
                attributeSize = sizeof( uint16_t ) * attribute.dimension;
                break;
            case VertexAttributeFormat::UNorm8:
            case VertexAttributeFormat::SNorm8:
            case VertexAttributeFormat::UInt8:
            case VertexAttributeFormat::SInt8:
                attributeSize = sizeof( uint8_t ) * attribute.dimension;
                break;
            default:
                break;
        }
        totalSize += attributeSize;
    }
    return totalSize * vertexCount;
}

godot::Image::Format getUncompressedPixelFormat( const CesiumGltf::ImageAsset &image )
{
    switch ( image.channels )
    {
        case 1:
            return godot::Image::FORMAT_L8;
            break;
        case 2:
            return godot::Image::FORMAT_LA8;
            break;
        case 3:
            return godot::Image::FORMAT_RGB8;
            break;
        case 4:
            return godot::Image::FORMAT_RGBA8;
            break;
        default:
            return godot::Image::FORMAT_RGBA8;
    }
}

godot::Image::Format getCompressedPixelFormat( const CesiumGltf::ImageAsset &image )
{
    switch ( image.compressedPixelFormat )
    {
        case GpuCompressedPixelFormat::ETC1_RGB:
            return godot::Image::Format::FORMAT_ETC;
        case GpuCompressedPixelFormat::ETC2_RGBA:
            return godot::Image::Format::FORMAT_ETC2_RGBA8;
        case GpuCompressedPixelFormat::BC1_RGB:
            return godot::Image::Format::FORMAT_RGB8;
        case GpuCompressedPixelFormat::BC3_RGBA:
            return godot::Image::Format::FORMAT_RGBA4444;
        case GpuCompressedPixelFormat::BC4_R:
            return godot::Image::Format::FORMAT_RGTC_R;
        case GpuCompressedPixelFormat::BC5_RG:
            return godot::Image::Format::FORMAT_RGTC_RG;
        case GpuCompressedPixelFormat::BC7_RGBA:
            return godot::Image::Format::FORMAT_BPTC_RGBA;
        case GpuCompressedPixelFormat::ASTC_4x4_RGBA:
            return godot::Image::Format::FORMAT_ASTC_4x4;
        case GpuCompressedPixelFormat::PVRTC1_4_RGB:
            return godot::Image::Format::FORMAT_RGB8;
        case GpuCompressedPixelFormat::PVRTC1_4_RGBA:
            return godot::Image::Format::FORMAT_RGBA4444;
        case GpuCompressedPixelFormat::ETC2_EAC_R11:
            return godot::Image::Format::FORMAT_ETC2_R11;
        case GpuCompressedPixelFormat::ETC2_EAC_RG11:
            return godot::Image::Format::FORMAT_ETC2_RG11;
        case GpuCompressedPixelFormat::PVRTC2_4_RGB:
        case GpuCompressedPixelFormat::PVRTC2_4_RGBA:
        default:
            return godot::Image::Format::FORMAT_RGBA8;
    }
}

Ref<godot::Image> loadImageFromCesiumImage( const CesiumGltf::ImageAsset &imageAsset, bool sRGB )
{
    int32_t width = imageAsset.width;
    int32_t height = imageAsset.height;
    int32_t channels = imageAsset.channels;
    godot::Image::Format format;
    Ref<godot::Image> image;

    if ( imageAsset.compressedPixelFormat == GpuCompressedPixelFormat::NONE )
    {
        format = getUncompressedPixelFormat( imageAsset );
    }
    else
    {
        format = getCompressedPixelFormat( imageAsset );
    }
    std::vector<uint8_t> pixelDataBuffer( width * height * channels * imageAsset.bytesPerChannel );

    if ( imageAsset.mipPositions.empty() )
    {
        std::memcpy( pixelDataBuffer.data(), imageAsset.pixelData.data(),
                     imageAsset.pixelData.size() );

        godot::PackedByteArray packedData;
        packedData.resize( pixelDataBuffer.size() );
        std::memcpy( packedData.ptrw(), pixelDataBuffer.data(), pixelDataBuffer.size() );
        image.instantiate();
        image->set_data( width, height, false, format, packedData );
    }
    else
    {
        size_t totalMipSize = 0;
        for ( const auto &mip : imageAsset.mipPositions )
        {
            totalMipSize += mip.byteSize;
        }

        if ( totalMipSize > pixelDataBuffer.size() )
        {
            pixelDataBuffer.resize( totalMipSize, 0 );
        }

        uint8_t *writePos = pixelDataBuffer.data();
        for ( const auto &mip : imageAsset.mipPositions )
        {
            std::memcpy( writePos, imageAsset.pixelData.data() + mip.byteOffset, mip.byteSize );
            writePos += mip.byteSize;
        }

        godot::PackedByteArray packedData;
        packedData.resize( imageAsset.mipPositions[0].byteSize );
        std::memcpy( packedData.ptrw(), pixelDataBuffer.data(),
                     imageAsset.mipPositions[0].byteSize );

        image.instantiate();
        image->set_data( width, height, false, format, packedData );
    }

    return image;
}

Ref<godot::ImageTexture> loadTexture( const CesiumGltf::Model &model, int32_t textureInfoIndex,
                                      bool sRGB )
{
    const CesiumGltf::Texture *pTexture = Model::getSafe( &model.textures, textureInfoIndex );
    const CesiumGltf::Image *pImage = CesiumGltf::Model::getSafe( &model.images, pTexture->source );
    if ( !pImage )
    {
        return Ref<godot::Texture>( nullptr );
    }

    const ImageAsset &imageAsset = *pImage->pAsset;
    Ref<godot::Image> image = loadImageFromCesiumImage( imageAsset, sRGB );

    Ref<ImageTexture> godotTexture = ImageTexture::create_from_image( image );

    return godotTexture;
}

static const CesiumGltf::MaterialPBRMetallicRoughness defaultPbrMetallicRoughness;
void setGltfMaterialParameterValues( const CesiumGltf::Model &model,
                                     const CesiumPrimitiveInfo &primitiveInfo,
                                     const CesiumGltf::Material &gltfMaterial,
                                     const Ref<StandardMaterial3D> material)
{

    CESIUM_TRACE( "Cesium::CreateMaterials" );
    const CesiumGltf::MaterialPBRMetallicRoughness &pbr =
        gltfMaterial.pbrMetallicRoughness ? gltfMaterial.pbrMetallicRoughness.value()
                                          : defaultPbrMetallicRoughness;

    // Add base color factor and metallic-roughness factor regardless
    // of whether the textures are present.
    const std::vector<double> &baseColorFactor = pbr.baseColorFactor;
    material->set_albedo(
        Color( baseColorFactor[0], baseColorFactor[1], baseColorFactor[2], baseColorFactor[3] ) );
    material->set_metallic( pbr.metallicFactor );
    material->set_roughness( pbr.roughnessFactor );

    const std::optional<CesiumGltf::TextureInfo> &baseColorTexture = pbr.baseColorTexture;
    if ( baseColorTexture )
    {
        auto texCoordIndexIt = primitiveInfo.uvIndexMap.find( baseColorTexture->texCoord );
        if ( texCoordIndexIt != primitiveInfo.uvIndexMap.end() )
        {
            Ref<godot::Texture> gTexture = loadTexture( model, baseColorTexture->index, true );
            if ( gTexture.is_valid() )
            {
                material->set_texture( StandardMaterial3D::TextureParam::TEXTURE_ALBEDO, gTexture );
                const CesiumGltf::Texture *pTexture =
                    Model::getSafe( &model.textures, baseColorTexture->index );
                const CesiumGltf::Sampler *pSampler =
                    CesiumGltf::Model::getSafe( &model.samplers, pTexture->source );
                if ( pSampler )
                {
                    if ( !pSampler->minFilter )
                    {
                        if ( pSampler->magFilter &&
                             *pSampler->magFilter == Sampler::MagFilter::NEAREST )
                        {
                            material->set_texture_filter(
                                BaseMaterial3D::TextureFilter::TEXTURE_FILTER_NEAREST );
                        }
                        else
                        {
                            material->set_texture_filter(
                                BaseMaterial3D::TextureFilter::TEXTURE_FILTER_LINEAR );
                        }
                    }
                    else
                    {
                        switch ( *pSampler->minFilter )
                        {
                            case Sampler::MinFilter::NEAREST:
                                material->set_texture_filter(
                                    BaseMaterial3D::TextureFilter::TEXTURE_FILTER_NEAREST );
                                break;
                            case Sampler::MinFilter::NEAREST_MIPMAP_NEAREST:
                                material->set_texture_filter(
                                    BaseMaterial3D::TextureFilter::
                                        TEXTURE_FILTER_NEAREST_WITH_MIPMAPS );
                                break;
                            case Sampler::MinFilter::LINEAR:
                                material->set_texture_filter(
                                    BaseMaterial3D::TextureFilter::TEXTURE_FILTER_LINEAR );
                                break;
                            case Sampler::MinFilter::LINEAR_MIPMAP_NEAREST:
                                material->set_texture_filter(
                                    BaseMaterial3D::TextureFilter::
                                        TEXTURE_FILTER_LINEAR_WITH_MIPMAPS );
                                break;
                            default:
                                material->set_texture_filter(
                                    BaseMaterial3D::TextureFilter::TEXTURE_FILTER_MAX );
                        }
                    }
                }
            }
        }
    }

    const std::optional<TextureInfo> &metallicRoughness = pbr.metallicRoughnessTexture;
    if ( metallicRoughness )
    {
        auto texCoordIndexIt = primitiveInfo.uvIndexMap.find( metallicRoughness->texCoord );
        if ( texCoordIndexIt != primitiveInfo.uvIndexMap.end() )
        {
            Ref<godot::Texture> gTexture = loadTexture( model, metallicRoughness->index, false );
            if ( gTexture.is_valid() )
            {
                material->set_texture( StandardMaterial3D::TextureParam::TEXTURE_METALLIC,
                                       gTexture );
            }
        }
    }

    const std::vector<double> &emissiveFactor = gltfMaterial.emissiveFactor;
    if ( gltfMaterial.emissiveTexture )
    {
        auto texCoordIndexIt =
            primitiveInfo.uvIndexMap.find( gltfMaterial.emissiveTexture->texCoord );
        if ( texCoordIndexIt != primitiveInfo.uvIndexMap.end() )
        {
            Ref<godot::Texture> gTexture =
                loadTexture( model, gltfMaterial.emissiveTexture->index, true );
            if ( gTexture.is_valid() )
            {
                material->set_texture( StandardMaterial3D::TextureParam::TEXTURE_EMISSION,
                                       gTexture );
            }
        }
    }

    if ( gltfMaterial.normalTexture )
    {
        auto texCoordIndexIt =
            primitiveInfo.uvIndexMap.find( gltfMaterial.normalTexture->texCoord );
        if ( texCoordIndexIt != primitiveInfo.uvIndexMap.end() )
        {
            Ref<godot::Texture> gTexture =
                loadTexture( model, gltfMaterial.normalTexture->index, true );
            if ( gTexture.is_valid() )
            {
                material->set_texture( StandardMaterial3D::TextureParam::TEXTURE_NORMAL, gTexture );
            }
        }
    }

    if ( gltfMaterial.occlusionTexture )
    {
        auto texCoordIndexIt =
            primitiveInfo.uvIndexMap.find( gltfMaterial.occlusionTexture->texCoord );
        if ( texCoordIndexIt != primitiveInfo.uvIndexMap.end() )
        {
            Ref<godot::Texture> gTexture =
                loadTexture( model, gltfMaterial.occlusionTexture->index, true );
            if ( gTexture.is_valid() )
            {
                material->set_texture( StandardMaterial3D::TextureParam::TEXTURE_AMBIENT_OCCLUSION,
                                       gTexture );
            }
        }
    }

    // Handle KHR_texture_transform for each available texture.
    CesiumGltf::KhrTextureTransform textureTransform;
    const CesiumGltf::ExtensionKhrTextureTransform *pBaseColorTextureTransform =
        pbr.baseColorTexture
            ? pbr.baseColorTexture->getExtension<CesiumGltf::ExtensionKhrTextureTransform>()
            : nullptr;
    if ( pBaseColorTextureTransform )
    {
        textureTransform = CesiumGltf::KhrTextureTransform( *pBaseColorTextureTransform );
        if ( textureTransform.status() == CesiumGltf::KhrTextureTransformStatus::Valid )
        {
            const glm::dvec2 &scale = textureTransform.scale();
            const glm::dvec2 &offset = textureTransform.offset();
            material->set_uv1_scale( Vector3( scale[0], scale[1], 0 ) );
            material->set_uv1_offset( Vector3( offset[0], offset[1], 0 ) );
        }
    }

    const CesiumGltf::ExtensionKhrTextureTransform *pMetallicRoughnessTextureTransform =
        pbr.metallicRoughnessTexture
            ? pbr.metallicRoughnessTexture->getExtension<CesiumGltf::ExtensionKhrTextureTransform>()
            : nullptr;
    if ( pMetallicRoughnessTextureTransform )
    {
        textureTransform = CesiumGltf::KhrTextureTransform( *pMetallicRoughnessTextureTransform );
        if ( textureTransform.status() == CesiumGltf::KhrTextureTransformStatus::Valid )
        {
            const glm::dvec2 &scale = textureTransform.scale();
            const glm::dvec2 &offset = textureTransform.offset();
            material->set_uv1_scale( Vector3( scale[0], scale[1], 0 ) );
            material->set_uv1_offset( Vector3( offset[0], offset[1], 0 ) );
        }
    }

    const CesiumGltf::ExtensionKhrTextureTransform *pNormalTextureTransform =
        gltfMaterial.normalTexture
            ? gltfMaterial.normalTexture->getExtension<CesiumGltf::ExtensionKhrTextureTransform>()
            : nullptr;
    if ( pNormalTextureTransform )
    {
        textureTransform = CesiumGltf::KhrTextureTransform( *pNormalTextureTransform );
        if ( textureTransform.status() == CesiumGltf::KhrTextureTransformStatus::Valid )
        {
            const glm::dvec2 &scale = textureTransform.scale();
            const glm::dvec2 &offset = textureTransform.offset();
            material->set_uv1_scale( Vector3( scale[0], scale[1], 0 ) );
            material->set_uv1_offset( Vector3( offset[0], offset[1], 0 ) );
        }
    }

    const CesiumGltf::ExtensionKhrTextureTransform *pEmissiveTextureTransform =
        gltfMaterial.emissiveTexture
            ? gltfMaterial.emissiveTexture->getExtension<CesiumGltf::ExtensionKhrTextureTransform>()
            : nullptr;
    if ( pEmissiveTextureTransform )
    {
        textureTransform = CesiumGltf::KhrTextureTransform( *pEmissiveTextureTransform );
        if ( textureTransform.status() == CesiumGltf::KhrTextureTransformStatus::Valid )
        {
            const glm::dvec2 &scale = textureTransform.scale();
            const glm::dvec2 &offset = textureTransform.offset();
            material->set_uv1_scale( Vector3( scale[0], scale[1], 0 ) );
            material->set_uv1_offset( Vector3( offset[0], offset[1], 0 ) );
        }
    }

    const CesiumGltf::ExtensionKhrTextureTransform *pOcclusionTextureTransform =
        gltfMaterial.occlusionTexture
            ? gltfMaterial.occlusionTexture
                  ->getExtension<CesiumGltf::ExtensionKhrTextureTransform>()
            : nullptr;
    if ( pOcclusionTextureTransform )
    {
        textureTransform = CesiumGltf::KhrTextureTransform( *pOcclusionTextureTransform );
        if ( textureTransform.status() == CesiumGltf::KhrTextureTransformStatus::Valid )
        {
            const glm::dvec2 &scale = textureTransform.scale();
            const glm::dvec2 &offset = textureTransform.offset();
            material->set_uv1_scale( Vector3( scale[0], scale[1], 0 ) );
            material->set_uv1_offset( Vector3( offset[0], offset[1], 0 ) );
        }
    }
}

void extractVertexData( const std::vector<uint8_t> &vertexData,
                        const VertexAttributeDescriptor *descriptors,
                        const int32_t numberOfAttributes, std::vector<glm::vec3> &positions,
                        std::vector<glm::vec3> &normals, std::vector<uint32_t> &colors,
                        std::vector<glm::vec2> &uvs )
{
    size_t stride = 0;
    for ( int32_t i = 0; i < numberOfAttributes; ++i )
    {
        const VertexAttributeDescriptor &desc = descriptors[i];
        size_t attributeSize = 0;
        switch ( desc.format )
        {
            case VertexAttributeFormat::Float32:
            case VertexAttributeFormat::UInt32:
            case VertexAttributeFormat::SInt32:
                attributeSize = sizeof( float ) * desc.dimension;
                break;
            case VertexAttributeFormat::Float16:
            case VertexAttributeFormat::UNorm16:
            case VertexAttributeFormat::SNorm16:
            case VertexAttributeFormat::UInt16:
            case VertexAttributeFormat::SInt16:
                attributeSize = sizeof( uint16_t ) * desc.dimension;
                break;
            case VertexAttributeFormat::UNorm8:
            case VertexAttributeFormat::SNorm8:
            case VertexAttributeFormat::UInt8:
            case VertexAttributeFormat::SInt8:
                attributeSize = sizeof( uint8_t ) * desc.dimension;
                break;
            default:
                break;
        }
        stride += attributeSize;
    }

    const uint8_t *pData = vertexData.data();
    size_t vertexCount = vertexData.size() / stride;
    positions.resize( vertexCount );
    colors.resize( vertexCount, 0xFFFFFFFF ); // set white color default if no colors
    normals.resize( vertexCount );
    uvs.resize( vertexCount );
    for ( size_t i = 0; i < vertexCount; ++i )
    {
        const uint8_t *pVertex = pData + i * stride;
        for ( int32_t j = 0; j < numberOfAttributes; ++j )
        {
            const VertexAttributeDescriptor &desc = descriptors[j];
            switch ( desc.attribute )
            {
                case VertexAttribute::Position:
                {
                    assert( desc.format == VertexAttributeFormat::Float32 && desc.dimension == 3 );
                    glm::vec3 pos;
                    std::memcpy( &pos, pVertex, sizeof( pos ) );
                    positions[i] = pos;
                    pVertex += sizeof( pos );
                    break;
                }
                case VertexAttribute::Normal:
                {
                    assert( desc.format == VertexAttributeFormat::Float32 && desc.dimension == 3 );
                    glm::vec3 norm;
                    std::memcpy( &norm, pVertex, sizeof( norm ) );
                    normals[i] = norm;
                    pVertex += sizeof( norm );
                    break;
                }
                case VertexAttribute::Color:
                {
                    assert( desc.format == VertexAttributeFormat::UNorm8 && desc.dimension == 4 );
                    uint32_t color;
                    std::memcpy( &color, pVertex, sizeof( color ) );
                    colors[i] = color;
                    pVertex += sizeof( color );
                    break;
                }
                case VertexAttribute::TexCoord0:
                {
                    assert( desc.format == VertexAttributeFormat::Float32 && desc.dimension == 2 );
                    glm::vec2 uv;
                    std::memcpy( &uv, pVertex, sizeof( uv ) );
                    uvs[i] = uv;
                    pVertex += sizeof( uv );
                    break;
                }
                default:
                    break;
            }
        }
    }
}

template <typename TIndex, class TIndexAccessor>
void loadPrimitive( Ref<ArrayMesh> arrMesh, CesiumPrimitiveInfo &primitiveInfo,
                    const CesiumGltf::Model &gltf, const CesiumGltf::Node &node,
                    const CesiumGltf::Mesh &mesh, const MeshPrimitive &primitive,
                    const glm::dmat4 &transform, const TIndexAccessor &indicesView,
                    const IndexFormat indexFormat, const AccessorView<glm::vec3> &positionView )
{

    CESIUM_TRACE( "Cesium::loadPrimitive<T>" );
    int32_t indexCount = 0;
    switch ( primitive.mode )
    {
        case MeshPrimitive::Mode::TRIANGLES:
        case MeshPrimitive::Mode::POINTS:
            indexCount = static_cast<int32_t>( indicesView.size() );
            break;
        case MeshPrimitive::Mode::TRIANGLE_STRIP:
        case MeshPrimitive::Mode::TRIANGLE_FAN:
            indexCount = static_cast<int32_t>( 3 * ( indicesView.size() - 2 ) );
            break;
        default:
            return;
    }

    if ( indexCount < 3 && primitive.mode != MeshPrimitive::Mode::POINTS )
    {
        return;
    }

    const CesiumGltf::Material *pMaterial =
        CesiumGltf::Model::getSafe( &gltf.materials, primitive.material );

    primitiveInfo.isUnlit = pMaterial && pMaterial->hasExtension<ExtensionKhrMaterialsUnlit>();

    bool hasNormals = false;
    bool shouldComputeFlatNormals = false;
    auto normalAccessorIt = primitive.attributes.find( "NORMAL" );
    AccessorView<glm::vec3> normalView;
    if ( normalAccessorIt != primitive.attributes.end() )
    {
        normalView = AccessorView<glm::vec3>( gltf, normalAccessorIt->second );
        hasNormals = normalView.status() == AccessorViewStatus::Valid;
    }
    else if ( !primitiveInfo.isUnlit && primitive.mode != MeshPrimitive::Mode::POINTS )
    {
        shouldComputeFlatNormals = hasNormals = true;
        SPDLOG_INFO( "Invalid normal buffer. Flat normals will be auto-generated instead." );
    }

    // Check if  we need to upgrade to a large index type to accommodate the
    // larger number of vertices we need for flat normals.
    if ( shouldComputeFlatNormals && indexFormat == IndexFormat::UInt16 &&
         indexCount >= std::numeric_limits<uint16_t>::max() )
    {
        loadPrimitive<uint32_t>( arrMesh, primitiveInfo, gltf, node, mesh, primitive, transform,
                                 indicesView, IndexFormat::UInt32, positionView );
        return;
    }

    std::vector<TIndex> indices;
    indices.resize( indexCount );
    TIndex *indicesData = indices.data();

    switch ( primitive.mode )
    {
        case MeshPrimitive::Mode::TRIANGLES:
        case MeshPrimitive::Mode::POINTS:
            for ( int32_t i = 0; i < indicesView.size(); ++i )
            {
                indicesData[i] = indicesView[i];
            }
            break;
        case MeshPrimitive::Mode::TRIANGLE_STRIP:
            for ( int32_t i = 0; i < indicesView.size() - 2; ++i )
            {
                if ( i % 2 )
                {
                    indicesData[3 * i] = indicesView[i];
                    indicesData[3 * i + 1] = indicesView[i + 2];
                    indicesData[3 * i + 2] = indicesView[i + 1];
                }
                else
                {
                    indicesData[3 * i] = indicesView[i];
                    indicesData[3 * i + 1] = indicesView[i + 1];
                    indicesData[3 * i + 2] = indicesView[i + 2];
                }
            }
            break;
        case CesiumGltf::MeshPrimitive::Mode::TRIANGLE_FAN:
        default:
            for ( int32_t i = 2; i < indicesView.size(); ++i )
            {
                indicesData[3 * i] = indicesView[0];
                indicesData[3 * i + 1] = indicesView[i - 1];
                indicesData[3 * i + 2] = indicesView[i];
            }
            break;
    }

    const int32_t MAX_ATTRIBUTES = 14;
    VertexAttributeDescriptor descriptors[MAX_ATTRIBUTES];

    // Interleave all attributes into single stream.
    int32_t numberOfAttributes = 0;

    descriptors[numberOfAttributes].attribute = VertexAttribute::Position;
    descriptors[numberOfAttributes].format = VertexAttributeFormat::Float32;
    descriptors[numberOfAttributes].dimension = 3;
    ++numberOfAttributes;

    if ( hasNormals )
    {
        assert( numberOfAttributes < MAX_ATTRIBUTES );
        descriptors[numberOfAttributes].attribute = VertexAttribute::Normal;
        descriptors[numberOfAttributes].format = VertexAttributeFormat::Float32;
        descriptors[numberOfAttributes].dimension = 3;
        ++numberOfAttributes;
    }

    bool needsTangents = hasNormals;
    bool hasTangents = false;
    auto tangentAccessorIt = primitive.attributes.find( "TANGENT" );
    if ( tangentAccessorIt != primitive.attributes.end() )
    {
        int32_t tangentAccessorID = tangentAccessorIt->second;
        auto tangentAccessor = AccessorView<glm::vec4>( gltf, tangentAccessorID );
        hasTangents = tangentAccessor.status() == AccessorViewStatus::Valid;
        if ( !hasTangents )
        {
            SPDLOG_INFO( "Invalid tangent buffer." );
        }
    }

    // Add the COLOR_0 attribute, if it exists.
    auto colorAccessorIt = primitive.attributes.find( "COLOR_0" );
    bool hasVertexColors =
        colorAccessorIt != primitive.attributes.end() &&
        validateVertexColors( gltf, colorAccessorIt->second, positionView.size() );
    if ( hasVertexColors )
    {
        assert( numberOfAttributes < MAX_ATTRIBUTES );

        descriptors[numberOfAttributes].attribute = VertexAttribute::Color;
        descriptors[numberOfAttributes].format = VertexAttributeFormat::UNorm8;
        descriptors[numberOfAttributes].dimension = 4;
        ++numberOfAttributes;

        const int8_t numComponents =
            gltf.accessors[colorAccessorIt->second].computeNumberOfComponents();
        if ( numComponents == 4 )
        {
            primitiveInfo.isTranslucent = true;
        }
    }

    constexpr int MAX_TEX_COORDS = 8;
    uint32_t numTexCoords = 0;
    AccessorView<glm::vec2> texCoordViews[MAX_TEX_COORDS];

    // Add all texture coordinate sets TEXCOORD_i
    for ( int i = 0; i < 8 && numTexCoords < MAX_TEX_COORDS; ++i )
    {
        // Build accessor view for glTF attribute.
        auto texCoordAccessorIt = primitive.attributes.find( "TEXCOORD_" + std::to_string( i ) );
        if ( texCoordAccessorIt == primitive.attributes.end() )
        {
            continue;
        }
        AccessorView<glm::vec2> texCoordView( gltf, texCoordAccessorIt->second );
        if ( texCoordView.status() != AccessorViewStatus::Valid &&
             texCoordView.size() >= positionView.size() )
        {
            continue;
        }

        texCoordViews[numTexCoords] = texCoordView;
        primitiveInfo.uvIndexMap[i] = numTexCoords;

        assert( numberOfAttributes < MAX_ATTRIBUTES );

        descriptors[numberOfAttributes].attribute =
            (VertexAttribute)( (int)VertexAttribute::TexCoord0 + numTexCoords );
        descriptors[numberOfAttributes].format = VertexAttributeFormat::Float32;
        descriptors[numberOfAttributes].dimension = 2;

        ++numTexCoords;
        ++numberOfAttributes;
    }

    // Add all texture coordinate sets _CESIUMOVERLAY_i
    for ( int i = 0; i < 8 && numTexCoords < MAX_TEX_COORDS; ++i )
    {
        // Build accessor view for glTF attribute.
        auto overlayAccessorIt =
            primitive.attributes.find( "_CESIUMOVERLAY_" + std::to_string( i ) );
        if ( overlayAccessorIt == primitive.attributes.end() )
        {
            continue;
        }

        AccessorView<glm::vec2> overlayTexCoordView( gltf, overlayAccessorIt->second );
        if ( overlayTexCoordView.status() != AccessorViewStatus::Valid &&
             overlayTexCoordView.size() >= positionView.size() )
        {
            continue;
        }

        texCoordViews[numTexCoords] = overlayTexCoordView;
        primitiveInfo.rasterOverlayUvIndexMap[i] = numTexCoords;

        assert( numberOfAttributes < MAX_ATTRIBUTES );
        descriptors[numberOfAttributes].attribute =
            (VertexAttribute)( (int)VertexAttribute::TexCoord0 + numTexCoords );
        descriptors[numberOfAttributes].format = VertexAttributeFormat::Float32;
        descriptors[numberOfAttributes].dimension = 2;

        ++numTexCoords;
        ++numberOfAttributes;
    }

    int32_t vertexCount =
        shouldComputeFlatNormals ? indexCount : static_cast<int32_t>( positionView.size() );
    std::vector<uint8_t> vertexData;
    vertexData.resize( computeVertexDataSize( vertexCount, descriptors, numberOfAttributes ) );
    uint8_t *pBufferStart = vertexData.data();
    uint8_t *pWritePos = pBufferStart;

    // Since the vertex buffer is dynamically interleaved, we don't have a
    // convenient struct to represent the vertex data.
    // The vertex layout will be as follows:
    // 1. position
    // 2. normals (skip if N/A)
    // 3. vertex colors (skip if N/A)
    // 4. texcoords (first all TEXCOORD_i, then all _CESIUMOVERLAY_i)
    size_t stride = sizeof( glm::vec3 );
    size_t normalByteOffset, colorByteOffset;
    if ( hasNormals )
    {
        normalByteOffset = stride;
        stride += sizeof( glm::vec3 );
    }

    if ( hasVertexColors )
    {
        colorByteOffset = stride;
        stride += sizeof( uint32_t );
    }
    stride += numTexCoords * sizeof( glm::vec2 );

    if ( shouldComputeFlatNormals )
    {
        computeFlatNormals( pWritePos + normalByteOffset, stride, indicesData, indexCount,
                            positionView );
        for ( int64_t i = 0; i < vertexCount; ++i )
        {
            TIndex vertexIndex = indicesData[i];
            *reinterpret_cast<glm::vec3 *>( pWritePos ) = positionView[vertexIndex];
            //   skip position and normal
            pWritePos += 2 * sizeof( glm::vec3 );
            // Skip the slot allocated for vertex colors, we will fill them in
            //  bulk later.
            if ( hasVertexColors )
            {
                pWritePos += sizeof( uint32_t );
            }
            for ( uint32_t texCoordIndex = 0; texCoordIndex < numTexCoords; ++texCoordIndex )
            {
                *reinterpret_cast<glm::vec2 *>( pWritePos ) =
                    texCoordViews[texCoordIndex][vertexIndex];
                pWritePos += sizeof( glm::vec2 );
            }
        }
    }
    else
    {
        for ( int64_t i = 0; i < vertexCount; ++i )
        {
            *reinterpret_cast<glm::vec3 *>( pWritePos ) = positionView[i];
            pWritePos += sizeof( glm::vec3 );

            if ( hasNormals )
            {
                *reinterpret_cast<glm::vec3 *>( pWritePos ) = normalView[i];
                pWritePos += sizeof( glm::vec3 );
            }

            // Skip the slot allocated for vertex colors, we will fill them in
            // bulk later.
            if ( hasVertexColors )
            {
                pWritePos += sizeof( uint32_t );
            }

            for ( uint32_t texCoordIndex = 0; texCoordIndex < numTexCoords; ++texCoordIndex )
            {
                *reinterpret_cast<glm::vec2 *>( pWritePos ) = texCoordViews[texCoordIndex][i];
                pWritePos += sizeof( glm::vec2 );
            }
        }
    }

    // Fill in vertex colors separately, createAccessorView if they exist.
    if ( hasVertexColors )
    {
        // Color comes after position and normal
        createAccessorView( gltf, colorAccessorIt->second,
                            CopyVertexColors<TIndex>{ pBufferStart + colorByteOffset, stride,
                                                      static_cast<size_t>( vertexCount ),
                                                      shouldComputeFlatNormals, indicesData } );
    }

    if ( shouldComputeFlatNormals )
    {
        // rewrite indices
        for ( int32_t i = 0; i < indexCount; ++i )
        {
            indicesData[i] = i;
        }
    }

    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<uint32_t> colors;
    std::vector<glm::vec2> uvs;
    extractVertexData( vertexData, descriptors, numberOfAttributes, positions, normals, colors,
                       uvs );

    //----------------------------------------------------------------------------
    PackedInt32Array indices_;
    PackedVector3Array verts_;
    PackedVector2Array uvs_;
    PackedVector3Array normals_;

    indices_.resize( indices.size() );
    for ( size_t i = 0, len = indices.size(); i < len; ++i )
    {
        indices_.set( i, static_cast<uint64_t>( indices[i] ) );
    }
    // Note: Godot uses clockwise winding order for front faces of triangle primitive modes.
    // need to reverse indices or it won't render correctly.
    indices_.reverse();

    verts_.resize( positions.size() );
    size_t i = 0;
    for ( glm::vec3 &vec3 : positions )
    {
        verts_.set( i, Vector3( vec3.x, vec3.y, vec3.z ) );
        ++i;
    }

    normals_.resize( normals.size() );
    i = 0;
    for ( glm::vec3 &vec3 : normals )
    {
        normals_.set( i, Vector3( vec3.x, vec3.y, vec3.z ) );
        ++i;
    }

    uvs_.resize( uvs.size() );
    i = 0;
    for ( glm::vec2 &vec2 : uvs )
    {
        uvs_.set( i, Vector2( vec2.x, vec2.y ) );
        ++i;
    }

    Array surface_array;
    surface_array.resize( ArrayMesh::ARRAY_MAX );
    surface_array[ArrayMesh::ARRAY_VERTEX] = verts_;
    surface_array[ArrayMesh::ARRAY_INDEX] = indices_;
    if ( hasNormals || shouldComputeFlatNormals )
    {
        surface_array[ArrayMesh::ARRAY_NORMAL] = normals_;
    }
    surface_array[ArrayMesh::ARRAY_TEX_UV] = uvs_;
    arrMesh->add_surface_from_arrays( ArrayMesh::PRIMITIVE_TRIANGLES, surface_array );
}

void populateMeshDataArray( std::vector<Ref<ArrayMesh>> &aMeshes,
                            std::vector<CesiumPrimitiveInfo> &primitiveInfos,
                            CesiumGltf::Model *pModel )
{
    int32_t numberOfPrimitives = countPrimitives( *pModel );
    aMeshes.reserve( numberOfPrimitives );
    primitiveInfos.reserve( numberOfPrimitives );

    pModel->forEachPrimitiveInScene(
        pModel->scene, [&aMeshes, &primitiveInfos, pModel](
                           const CesiumGltf::Model &gltf, const CesiumGltf::Node &node,
                           const CesiumGltf::Mesh &mesh, const CesiumGltf::MeshPrimitive &primitive,
                           const glm::dmat4 &transform ) {
            Ref<ArrayMesh> &aMesh = aMeshes.emplace_back();
            aMesh.instantiate();
            CesiumPrimitiveInfo &primitiveInfo = primitiveInfos.emplace_back();
            auto positionAccessorIt = primitive.attributes.find( "POSITION" );
            if ( positionAccessorIt == primitive.attributes.end() )
            {
                return;
            }
            int32_t positionAccessorID = positionAccessorIt->second;
            AccessorView<glm::vec3> positionView( gltf, positionAccessorID );
            if ( positionView.status() != AccessorViewStatus::Valid )
            {
                return;
            }

            generateMipMapsForPrimitive( pModel, primitive );

            if ( primitive.indices < 0 || primitive.indices >= gltf.accessors.size() )
            {
                int32_t indexCount = static_cast<int32_t>( positionView.size() );
                if ( indexCount > std::numeric_limits<std::uint16_t>::max() )
                {
                    loadPrimitive<std::uint32_t>( aMesh, primitiveInfo, gltf, node, mesh, primitive,
                                                  transform,
                                                  generateIndices<std::uint32_t>( indexCount ),
                                                  IndexFormat::UInt32, positionView );
                }
                else
                {
                    loadPrimitive<std::uint16_t>( aMesh, primitiveInfo, gltf, node, mesh, primitive,
                                                  transform,
                                                  generateIndices<std::uint16_t>( indexCount ),
                                                  IndexFormat::UInt16, positionView );
                }
            }
            else
            {
                const Accessor &indexAccessorGltf = gltf.accessors[primitive.indices];
                switch ( indexAccessorGltf.componentType )
                {
                    case Accessor::ComponentType::BYTE:
                    {
                        AccessorView<int8_t> indexAccessor( gltf, primitive.indices );
                        loadPrimitive<std::uint16_t>( aMesh, primitiveInfo, gltf, node, mesh,
                                                      primitive, transform, indexAccessor,
                                                      IndexFormat::UInt16, positionView );
                        break;
                    }
                    case Accessor::ComponentType::UNSIGNED_BYTE:
                    {
                        AccessorView<uint8_t> indexAccessor( gltf, primitive.indices );
                        loadPrimitive<std::uint16_t>( aMesh, primitiveInfo, gltf, node, mesh,
                                                      primitive, transform, indexAccessor,
                                                      IndexFormat::UInt16, positionView );
                        break;
                    }
                    case Accessor::ComponentType::SHORT:
                    {
                        AccessorView<int16_t> indexAccessor( gltf, primitive.indices );
                        loadPrimitive<std::uint16_t>( aMesh, primitiveInfo, gltf, node, mesh,
                                                      primitive, transform, indexAccessor,
                                                      IndexFormat::UInt16, positionView );
                        break;
                    }
                    case Accessor::ComponentType::UNSIGNED_SHORT:
                    {
                        AccessorView<uint16_t> indexAccessor( gltf, primitive.indices );
                        loadPrimitive<std::uint16_t>( aMesh, primitiveInfo, gltf, node, mesh,
                                                      primitive, transform, indexAccessor,
                                                      IndexFormat::UInt16, positionView );
                        break;
                    }
                    case Accessor::ComponentType::UNSIGNED_INT:
                    {
                        AccessorView<uint32_t> indexAccessor( gltf, primitive.indices );
                        loadPrimitive<std::uint32_t>( aMesh, primitiveInfo, gltf, node, mesh,
                                                      primitive, transform, indexAccessor,
                                                      IndexFormat::UInt32, positionView );
                        break;
                    }
                    default:
                        return;
                }
            }
        } );
}

GodotPrepareRendererResources::GodotPrepareRendererResources( Cesium3DTileset *tileset ) :
    _tileset( tileset )
{
}

CesiumAsync::Future<Cesium3DTilesSelection::TileLoadResultAndRenderResources>
    GodotPrepareRendererResources::prepareInLoadThread(
        const CesiumAsync::AsyncSystem &asyncSystem,
        Cesium3DTilesSelection::TileLoadResult &&tileLoadResult, const glm::dmat4 &transform,
        const std::any &rendererOptions )
{
    CesiumGltf::Model *pModel = std::get_if<CesiumGltf::Model>( &tileLoadResult.contentKind );
    if ( !pModel )
    {
        return asyncSystem.createResolvedFuture(
            TileLoadResultAndRenderResources{ std::move( tileLoadResult ), nullptr } );
    }
    std::vector<Ref<ArrayMesh>> meshes{};
    std::vector<CesiumPrimitiveInfo> primitiveInfos{};
    populateMeshDataArray( meshes, primitiveInfos, pModel );

    LoadThreadResult *pResult = new LoadThreadResult{ meshes, std::move( primitiveInfos ) };
    return asyncSystem.createResolvedFuture(
        TileLoadResultAndRenderResources{ std::move( tileLoadResult ), pResult } );
}

void *GodotPrepareRendererResources::prepareInMainThread( Cesium3DTilesSelection::Tile &tile,
                                                          void *pLoadThreadResult_ )
{
    const Cesium3DTilesSelection::TileContent &content = tile.getContent();
    const Cesium3DTilesSelection::TileRenderContent *pRenderContent = content.getRenderContent();
    if ( !pRenderContent )
    {
        return nullptr;
    }

    std::unique_ptr<LoadThreadResult> pLoadThreadResult(
        static_cast<LoadThreadResult *>( pLoadThreadResult_ ) );
    std::vector<Ref<ArrayMesh>> meshes = pLoadThreadResult->meshes;
    std::vector<CesiumPrimitiveInfo> primitiveInfos = pLoadThreadResult->primitiveInfos;

    const CesiumGltf::Model &model = pRenderContent->getModel();
    glm::dmat4 tileTransform = tile.getTransform();
    tileTransform = GltfUtilities::applyRtcCenter( model, tileTransform );
    GltfUtilities::applyGltfUpAxisTransform( model, tileTransform );

    int32_t meshSize = meshes.size();
    if ( !meshSize )
    {
        return nullptr;
    }

    std::string name = "glTF";
    auto urlIt = model.extras.find( "Cesium3DTiles_TileUrl" );
    if ( urlIt != model.extras.end() )
    {
        name = urlIt->second.getStringOrDefault( "glTF" );
    }

    std::vector<MeshInstance3D *> meshInstances;
    meshInstances.reserve( meshSize );
    for ( int i = 0; i < meshSize; ++i )
    {
        MeshInstance3D *meshInstance = memnew( MeshInstance3D );
        meshInstance->set_name( godot::String( name.c_str() ) );
        meshInstance->set_mesh( meshes[i] );
        // cesium coordinate axis X is not align godot's, need rotate.
        Quaternion qua( Vector3( 1, 0, 0 ), static_cast<real_t>( -Math_PI / 2 ) );
        Transform3D trans;
        trans.set_basis( qua );
        meshInstance->set_transform( trans );
        meshInstance->set_visible( false );
        this->_tileset->add_child( meshInstance );
        meshInstances.push_back( meshInstance );
    }

    const bool createPhysicsMeshes = this->_tileset->get_create_physics_meshes();

    int32_t meshIndex = 0;
    model.forEachPrimitiveInScene(
        model.scene, [&meshes, &meshIndex, &meshInstances, &primitiveInfos, &createPhysicsMeshes, model](
                         const CesiumGltf::Model &gltf, const CesiumGltf::Node &node,
                         const CesiumGltf::Mesh &mesh, const CesiumGltf::MeshPrimitive &primitive,
                         const glm::dmat4 &transform ) {
            const CesiumPrimitiveInfo &primitiveInfo = primitiveInfos[meshIndex];
            Ref<ArrayMesh> ArrayMeshRef = meshes[meshIndex];
            MeshInstance3D *meshInstance = meshInstances[meshIndex];
            meshIndex++;
            auto positionAccessorIt = primitive.attributes.find( "POSITION" );
            if ( positionAccessorIt == primitive.attributes.end() )
            {
                // This primitive doesn't have a POSITION semantic, ignore it.
                return;
            }

            int32_t positionAccessorID = positionAccessorIt->second;
            AccessorView<Vector3> positionView( gltf, positionAccessorID );
            if ( positionView.status() != AccessorViewStatus::Valid )
            {
                return;
            }

            Ref<StandardMaterial3D> material;
            material.instantiate();

            const CesiumGltf::Material *pMaterial =
                Model::getSafe( &gltf.materials, primitive.material );
            if ( pMaterial )
            {
                setGltfMaterialParameterValues( gltf, primitiveInfo, *pMaterial, material);
            }
            meshInstance->set_material_override( material );

            if ( primitiveInfo.containsPoints )
            {
                // TODO: pointClound

                return;
            }

            if ( createPhysicsMeshes )
            {
                if ( !godot::Engine::get_singleton()->is_editor_hint() &&
                     !isDegenerateTriangleMesh( ArrayMeshRef ) )
                {

                    meshInstance->create_convex_collision();
                }
            }
        } );

    return new CesiumGltfNode{ std::move( meshInstances ),
                               std::move( pLoadThreadResult->primitiveInfos ), false };
}

void GodotPrepareRendererResources::free( Cesium3DTilesSelection::Tile &tile,
                                          void *pLoadThreadResult,
                                          void *pMainThreadResult ) noexcept
{
    if (this->_tileset->tiles_destroyed)
    {
        return;
    }
    SPDLOG_INFO("prepare to free resources");
    if ( pLoadThreadResult )
    {
        LoadThreadResult *result = static_cast<LoadThreadResult *>( pLoadThreadResult );
        result->meshes.clear();
        delete result;
    }
    if ( pMainThreadResult )
    {
        CesiumGltfNode *pGltfNode = static_cast<CesiumGltfNode *>( pMainThreadResult );
        if (!pGltfNode->isFreed)
        {
            for ( MeshInstance3D *meshInstance : pGltfNode->pNodes )
            {
                if (meshInstance && meshInstance->is_inside_tree())
                {
                    godot::Node *parent = meshInstance->get_parent();
                    if (parent)
                    {
                        parent->remove_child(meshInstance);
                    }
                    // 由Godot负责内存释放，避免手动delete
                    meshInstance->queue_free();
                }
            }
            pGltfNode->pNodes.clear();
            pGltfNode->isFreed = true;
            delete pGltfNode;
        } else{
            SPDLOG_WARN("pGltfNode: {} already freed!", (void*)pGltfNode);
        }
    }
}

void *GodotPrepareRendererResources::prepareRasterInLoadThread( CesiumGltf::ImageAsset &image,
                                                                const std::any &rendererOptions )
{
    return nullptr;
}

void *GodotPrepareRendererResources::prepareRasterInMainThread(
    CesiumRasterOverlays::RasterOverlayTile &rasterTile, void *pLoadThreadResult )
{
    return nullptr;
}

void GodotPrepareRendererResources::freeRaster(
    const CesiumRasterOverlays::RasterOverlayTile &rasterTile, void *pLoadThreadResult,
    void *pMainThreadResult ) noexcept
{
}

void GodotPrepareRendererResources::attachRasterInMainThread(
    const Cesium3DTilesSelection::Tile &tile, int32_t overlayTextureCoordinateID,
    const CesiumRasterOverlays::RasterOverlayTile &rasterTile, void *pMainThreadRendererResources,
    const glm::dvec2 &translation, const glm::dvec2 &scale )
{
}

void GodotPrepareRendererResources::detachRasterInMainThread(
    const Cesium3DTilesSelection::Tile &tile, int32_t overlayTextureCoordinateID,
    const CesiumRasterOverlays::RasterOverlayTile &rasterTile,
    void *pMainThreadRendererResources ) noexcept
{
}
