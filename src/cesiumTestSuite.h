#ifndef _CESIUM_TEST_SUITE_
#define _CESIUM_TEST_SUITE_

#include "CesiumGltf/AccessorView.h"
#include "CesiumGltf/Model.h"
#include <godot_cpp/classes/array_mesh.hpp>
#include <godot_cpp/classes/camera3d.hpp>
#include <godot_cpp/classes/editor_interface.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/gltf_accessor.hpp>
#include <godot_cpp/classes/gltf_buffer_view.hpp>
#include <godot_cpp/classes/gltf_mesh.hpp>
#include <godot_cpp/classes/gltf_node.hpp>
#include <godot_cpp/classes/gltf_state.hpp>
#include <godot_cpp/classes/image.hpp>
#include <godot_cpp/classes/image_texture.hpp>
#include <godot_cpp/classes/importer_mesh.hpp>
#include <godot_cpp/classes/mesh_instance3d.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/random_number_generator.hpp>
#include <godot_cpp/classes/standard_material3d.hpp>
#include <godot_cpp/classes/sub_viewport.hpp>
#include <godot_cpp/classes/viewport.hpp>
#include <godot_cpp/templates/vector.hpp>
#include <godot_cpp/variant/color.hpp>
#include <godot_cpp/variant/packed_int32_array.hpp>
#include <godot_cpp/variant/packed_vector2_array.hpp>
#include <godot_cpp/variant/packed_vector3_array.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/variant/variant.hpp>
#include <godot_cpp/variant/vector2.hpp>
#include <godot_cpp/variant/vector3.hpp>

namespace CesiumForGodot
{

    CesiumGltf::Model createCubeGltf();

    godot::Ref<godot::GLTFState> convertCesiumGltfToGodotGLTFState( CesiumGltf::Model &model );

    godot::MeshInstance3D *createTriangle();

    godot::MeshInstance3D *createCube();

    godot::Ref<godot::ImageTexture> loadTextureFromFile( const godot::String &path );

    godot::Ref<godot::ImageTexture> createRandomTexture( int width, int height );

    void testEditorCameraStates();

    void createMatrixOfCubes( godot::Vector<godot::MeshInstance3D *> &instances );

    const int GRID_SIZE = 10;
    const int SPACING = 100;
    const int CUBE_SIZE = 10;

} // namespace CesiumForGodot

#endif