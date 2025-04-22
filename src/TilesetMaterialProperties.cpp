#include "TilesetMaterialProperties.h"
#include <godot_cpp/classes/base_material3d.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include <unordered_set>

using namespace CesiumForGodot;
using namespace godot;

#pragma region Parameter Names
const std::string TilesetMaterialProperties::_overlayTexturePrefix = "_overlay_texture_";
const std::string TilesetMaterialProperties::_overlayTextureCoordinateIndexPrefix =
    "_overlay_texture_coordinate_index_";
const std::string TilesetMaterialProperties::_overlayTranslationAndScalePrefix =
    "_overlay_translation_and_scale_";
#pragma endregion

TilesetMaterialProperties::TilesetMaterialProperties() :
    _overlayTextureCoordinateIndexIDs(), _overlayTextureIDs(), _overlayTranslationAndScaleIDs()
{
}

const std::optional<std::string> TilesetMaterialProperties::getOverlayTextureCoordinateIndexID(
    const std::string &key ) const noexcept
{
    auto iter = this->_overlayTextureCoordinateIndexIDs.find( key );
    if ( iter == this->_overlayTextureCoordinateIndexIDs.end() )
    {
        return std::nullopt;
    }
    return this->_overlayTextureCoordinateIndexIDs.at( key );
}

const std::optional<std::string> TilesetMaterialProperties::getOverlayTextureID(
    const std::string &key ) const noexcept
{
    auto iter = this->_overlayTextureIDs.find( key );
    if ( iter == this->_overlayTextureIDs.end() )
    {
        return std::nullopt;
    }
    return this->_overlayTextureIDs.at( key );
}

const std::optional<std::string> TilesetMaterialProperties::getOverlayTranslationAndScaleID(
    const std::string &key ) const noexcept
{
    auto iter = this->_overlayTranslationAndScaleIDs.find( key );
    if ( iter == this->_overlayTranslationAndScaleIDs.end() )
    {
        return std::nullopt;
    }
    return this->_overlayTranslationAndScaleIDs.at( key );
}

void TilesetMaterialProperties::updateOverlayParameterIDs(
    const std::vector<std::string> &overlayMaterialKeys )
{
    const size_t size = overlayMaterialKeys.size();
    this->_overlayTextureIDs.reserve( size );
    this->_overlayTextureCoordinateIndexIDs.reserve( size );
    this->_overlayTranslationAndScaleIDs.reserve( size );

    std::unordered_set<std::string> uniqueKeys;
    for ( size_t i = 0; i < size; i++ )
    {
        const std::string &key = overlayMaterialKeys[i];
        if ( uniqueKeys.find( key ) != uniqueKeys.end() )
        {
            UtilityFunctions::print_verbose( "Two or more raster overlays use the same material "
                                             "key on the same Cesium3DTileset. This will cause "
                                             "unexpected behavior, as only one of them will be "
                                             "passed to the tileset's material." );
            continue;
        }
        uniqueKeys.insert( key );

        this->_overlayTextureIDs.insert( { key, _overlayTexturePrefix + key } );
        this->_overlayTextureCoordinateIndexIDs.insert(
            { key, _overlayTextureCoordinateIndexPrefix + key } );
        this->_overlayTranslationAndScaleIDs.insert(
            { key, _overlayTranslationAndScalePrefix + key } );
    }
}
