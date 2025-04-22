#include "CameraManager.h"
#include "CesiumGeoreference.h"

#include <CesiumGeospatial/Ellipsoid.h>
#include <CesiumGeospatial/GlobeTransforms.h>
#include <CesiumUtility/Math.h>

#include <glm/trigonometric.hpp>

#include <godot_cpp/classes/camera3d.hpp>
#include <godot_cpp/classes/editor_interface.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/sub_viewport.hpp>
#include <godot_cpp/classes/viewport.hpp>

#include <array>

using namespace Cesium3DTilesSelection;
using namespace CesiumGeospatial;
using namespace CesiumUtility;

namespace CesiumForGodot
{

    namespace
    {

        ViewState godotCameraToViewState( const CesiumGeoreference *georeference,
                                          const LocalHorizontalCoordinateSystem *pCoordinateSystem,
                                          const glm::dmat4 &godotWorldToTileset,
                                          const godot::Camera3D *camera,
                                          const godot::Size2 viewportSize )
        {
            godot::Transform3D transform = camera->get_camera_transform();
            godot::Vector3 origin = transform.get_origin();

            glm::dvec3 cameraPosition =
                glm::dvec3( godotWorldToTileset * glm::dvec4( origin.x, origin.y, origin.z, 1.0 ) );

            godot::Vector3 cameraDirectionGodot = -transform.basis.get_column( 2 ); // column major
            glm::dvec3 cameraDirection = glm::dvec3(
                godotWorldToTileset * glm::dvec4( cameraDirectionGodot.x, cameraDirectionGodot.y,
                                                  cameraDirectionGodot.z, 0.0 ) );

            godot::Vector3 cameraUpGodot = transform.basis.get_column( 1 );
            glm::dvec3 cameraUp =
                glm::dvec3( godotWorldToTileset *
                            glm::dvec4( cameraUpGodot.x, cameraUpGodot.y, cameraUpGodot.z, 0.0 ) );

            if ( pCoordinateSystem != nullptr )
            {
                cameraPosition = pCoordinateSystem->localPositionToEcef( cameraPosition );
                cameraDirection = pCoordinateSystem->localDirectionToEcef( cameraDirection );
                cameraUp = pCoordinateSystem->localDirectionToEcef( cameraUp );
            }

            double verticalFOV = CesiumUtility::Math::degreesToRadians( camera->get_fov() );
            double width = viewportSize.width;
            double height = viewportSize.height;
            double horizontalFOV = 2 * glm::atan( width / height * glm::tan( verticalFOV * 0.5 ) );

            const CesiumGeospatial::Ellipsoid &ellipsoid =
                georeference != nullptr ? georeference->get_ellipsoid()->get_native_ellipsoid()
                                        : CesiumGeospatial::Ellipsoid::WGS84;

            return ViewState::create( cameraPosition, glm::normalize( cameraDirection ),
                                      glm::normalize( cameraUp ), glm::dvec2( width, height ),
                                      horizontalFOV, verticalFOV, ellipsoid );
        }

        glm::dmat4 godotTransform3DToGlm( godot::Transform3D transform )
        {
            // Transform3D basis is column major
            godot::Basis basis = transform.basis;
            godot::Vector3 translation = transform.origin;

            glm::mat3 glmBasis( basis[0][0], basis[1][0], basis[2][0], basis[0][1], basis[1][1],
                                basis[2][1], basis[0][2], basis[1][2], basis[2][2] );

            glm::vec3 glmTranslation( translation.x, translation.y, translation.z );
            glm::mat4 glmMat4( glmBasis );
            glmMat4[3] = glm::vec4( glmTranslation, 1.0f );

            glm::dmat4 glmDmat4;
            for ( int i = 0; i < 4; ++i )
            {
                for ( int j = 0; j < 4; ++j )
                {
                    glmDmat4[i][j] = static_cast<double>( glmMat4[i][j] );
                }
            }

            return glmDmat4;
        }

    } // namespace

    std::vector<ViewState> CameraManager::getAllCameras( const Cesium3DTileset &tileset )
    {
        const LocalHorizontalCoordinateSystem *pCoordinateSystem = nullptr;

        glm::dmat4 godotWorldToTileset = godotTransform3DToGlm( tileset.get_transform() );

        Node *parent = tileset.get_parent();
        CesiumGeoreference *georeference = nullptr;
        if ( parent )
        {
            georeference = Object::cast_to<CesiumGeoreference>( parent );
            if ( georeference )
            {
                const LocalHorizontalCoordinateSystem coordinateSystem =
                    georeference->getCoordinateSystem();
                pCoordinateSystem = &coordinateSystem;
            }
        }
        else
        {
            SPDLOG_ERROR( "Cesium3DTilset Node must be the subnode of CesiumGeoreference" );
        }

        std::vector<ViewState> result;
        godot::Camera3D *currentCamera = nullptr;
        godot::Viewport *viewport = tileset.get_viewport();
        if ( viewport != nullptr )
        {
            currentCamera = viewport->get_camera_3d();
        }

        if ( currentCamera != nullptr )
        {
            godot::Size2 viewportSize = viewport->get_visible_rect().size;
            if ( viewportSize.width > 50 && viewportSize.height > 50 )
            {
                result.emplace_back( godotCameraToViewState( georeference, pCoordinateSystem,
                                                             godotWorldToTileset, currentCamera,
                                                             viewportSize ) );
            }
        }

        if ( godot::Engine::get_singleton()->is_editor_hint() )
        {

            EditorInterface *editor_interface = EditorInterface::get_singleton();
            Node *editor_root_node = editor_interface->get_edited_scene_root();
            godot::SubViewport *editor_viewport = nullptr;
            godot::Camera3D *editor_camera = nullptr;

            std::array<int, 4> indices{ 0, 1, 2, 3 };
            for ( int i : indices )
            {
                editor_viewport = editor_interface->get_editor_viewport_3d( i );
                if ( editor_viewport != nullptr )
                {
                    editor_camera = editor_viewport->get_camera_3d();
                    if ( editor_camera != nullptr )
                    {
                        godot::Size2 viewportSize = editor_viewport->get_visible_rect().size;
                        if ( viewportSize.width > 50 && viewportSize.height > 50 )
                        {
                            result.emplace_back( godotCameraToViewState(
                                georeference, pCoordinateSystem, godotWorldToTileset, editor_camera,
                                viewportSize ) );
                        }
                    }
                }
            }
        }

        return result;
    }

} // namespace CesiumForGodot
