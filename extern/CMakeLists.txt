
# godot-cpp
# From here: https://github.com/godotengine/godot-cpp
if ( NOT EXISTS "${CMAKE_CURRENT_LIST_DIR}/godot-cpp/Makefile" )
    message(
        FATAL_ERROR
            "[${PROJECT_NAME}] The godot-cpp submodule was not downloaded. Please update submodules: git submodule update --init --recursive."
    )
endif()

set( GODOT_ENABLE_HOT_RELOAD ON)
set( HOT_RELOAD_ENABLED ON CACHE BOOL "" FORCE )
set( GODOT_CPP_SYSTEM_HEADERS ON CACHE BOOL "" FORCE )

add_subdirectory( godot-cpp EXCLUDE_FROM_ALL )

target_compile_definitions( godot-cpp PUBLIC HOT_RELOAD_ENABLED)

set_target_properties( godot-cpp
    PROPERTIES
        CXX_VISIBILITY_PRESET hidden    # visibility needs to be the same as the main library
)
    

# Cesium native wrapper 
add_subdirectory( cesium-native EXCLUDE_FROM_ALL )

add_library(cesium-native-wrapper INTERFACE)

target_link_libraries(cesium-native-wrapper INTERFACE
    CesiumUtility
    CesiumGltf
    CesiumGeometry
    CesiumGeospatial
    CesiumJsonReader
    CesiumJsonWriter
    CesiumGltfContent
    CesiumGltfReader
    CesiumGltfWriter
    CesiumAsync
    Cesium3DTiles
    Cesium3DTilesReader
    Cesium3DTilesWriter
    Cesium3DTilesContent
    CesiumRasterOverlays
    Cesium3DTilesSelection
    # CesiumIonClient
    # CesiumQuantizedMeshTerrain
)

target_link_libraries( ${PROJECT_NAME}
    PRIVATE
        godot-cpp
        cesium-native-wrapper
)