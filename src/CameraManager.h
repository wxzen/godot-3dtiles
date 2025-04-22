#ifndef CAMERA_MANAGER_H
#define CAMERA_MANAGER_H

#include <Cesium3DTilesSelection/ViewState.h>
#include <spdlog/spdlog.h>

#include <vector>

#include "Cesium3DTileset.h"

namespace CesiumForGodot
{

    class CameraManager
    {
    public:
        static std::vector<Cesium3DTilesSelection::ViewState> getAllCameras(
            const Cesium3DTileset &tileset );
    };

} // namespace CesiumForGodot

#endif