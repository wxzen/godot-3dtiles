#ifndef _TILESET_MATERIAL_PROPERTIES_
#define _TILESET_MATERIAL_PROPERTIES_

#include <cstdint>
#include <godot_cpp/variant/string.hpp>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace CesiumForGodot
{

    class TilesetMaterialProperties
    {
    public:
        TilesetMaterialProperties();

        const std::optional<std::string> getOverlayTextureCoordinateIndexID(
            const std::string &key ) const noexcept;
        const std::optional<std::string> getOverlayTextureID(
            const std::string &key ) const noexcept;
        const std::optional<std::string> getOverlayTranslationAndScaleID(
            const std::string &key ) const noexcept;

        void updateOverlayParameterIDs( const std::vector<std::string> &overlayMaterialKeys );

    private:
        std::unordered_map<std::string, std::string> _overlayTextureCoordinateIndexIDs;
        std::unordered_map<std::string, std::string> _overlayTextureIDs;
        std::unordered_map<std::string, std::string> _overlayTranslationAndScaleIDs;

        static const std::string _overlayTexturePrefix;
        static const std::string _overlayTextureCoordinateIndexPrefix;
        static const std::string _overlayTranslationAndScalePrefix;
    };

} // namespace CesiumForGodot

#endif