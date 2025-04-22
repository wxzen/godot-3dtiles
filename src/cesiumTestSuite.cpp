#include "cesiumTestSuite.h"
#include "Enums.hpp"

#include <iostream>

using namespace CesiumGltf;

namespace CesiumForGodot
{
    Model createCubeGltf()
    {
        Model model;

        std::vector<glm::vec3> cubeVertices = {
            glm::vec3( 0.0f, 0.0f, 0.0f ), glm::vec3( 1.0f, 0.0f, 0.0f ),
            glm::vec3( 1.0f, 0.0f, 1.0f ), glm::vec3( 0.0f, 0.0f, 1.0f ),

            glm::vec3( 0.0f, 1.0f, 0.0f ), glm::vec3( 1.0f, 1.0f, 0.0f ),
            glm::vec3( 1.0f, 1.0f, 1.0f ), glm::vec3( 0.0f, 1.0f, 1.0f )
        };

        // TODO: generalize type so each index type can be tested?
        std::vector<uint8_t> cubeIndices = { 0, 1, 2, 0, 2, 3,

                                             4, 6, 5, 4, 7, 6,

                                             0, 5, 1, 0, 4, 5,

                                             0, 7, 4, 0, 3, 7,

                                             1, 5, 6, 1, 6, 2,

                                             3, 2, 6, 3, 6, 7 };

        size_t vertexByteStride = sizeof( glm::vec3 );
        size_t vertexByteLength = 8 * vertexByteStride;

        Buffer &vertexBuffer = model.buffers.emplace_back();
        vertexBuffer.byteLength = static_cast<int64_t>( vertexByteLength );
        vertexBuffer.cesium.data.resize( vertexByteLength );
        std::memcpy( vertexBuffer.cesium.data.data(), &cubeVertices[0], vertexByteLength );

        BufferView &vertexBufferView = model.bufferViews.emplace_back();
        vertexBufferView.buffer = 0;
        vertexBufferView.byteLength = vertexBuffer.byteLength;
        vertexBufferView.byteOffset = 0;
        vertexBufferView.byteStride = static_cast<int64_t>( vertexByteStride );
        vertexBufferView.target = BufferView::Target::ARRAY_BUFFER;

        Accessor &vertexAccessor = model.accessors.emplace_back();
        vertexAccessor.bufferView = 0;
        vertexAccessor.byteOffset = 0;
        vertexAccessor.componentType = Accessor::ComponentType::FLOAT;
        vertexAccessor.count = 8;
        vertexAccessor.type = Accessor::Type::VEC3;

        Buffer &indexBuffer = model.buffers.emplace_back();
        indexBuffer.byteLength = 36;
        indexBuffer.cesium.data.resize( 36 );
        std::memcpy( indexBuffer.cesium.data.data(), &cubeIndices[0], 36 );

        BufferView &indexBufferView = model.bufferViews.emplace_back();
        indexBufferView.buffer = 1;
        indexBufferView.byteLength = 36;
        indexBufferView.byteOffset = 0;
        indexBufferView.byteStride = 1;
        indexBufferView.target = BufferView::Target::ELEMENT_ARRAY_BUFFER;

        Accessor &indexAccessor = model.accessors.emplace_back();
        indexAccessor.bufferView = 1;
        indexAccessor.byteOffset = 0;
        indexAccessor.componentType = Accessor::ComponentType::UNSIGNED_BYTE;
        indexAccessor.count = 36;
        indexAccessor.type = Accessor::Type::SCALAR;

        Scene &scene = model.scenes.emplace_back();
        Node &node = model.nodes.emplace_back();
        Mesh &mesh = model.meshes.emplace_back();
        MeshPrimitive &primitive = mesh.primitives.emplace_back();

        primitive.attributes.emplace( "POSITION", 0 );
        primitive.indices = 1;
        primitive.mode = MeshPrimitive::Mode::TRIANGLES;

        model.scene = 0;
        scene.nodes = { 0 };
        node.mesh = 0;

        return model;
    }

    godot::MeshInstance3D *createTriangle()
    {
        using namespace godot;
        PackedVector3Array verts;
        PackedInt32Array indices;
        PackedVector3Array normals;
        PackedVector2Array uvs;
        verts.push_back( Vector3( 1.0, 0.0, 1.0 ) );
        verts.push_back( Vector3( 1.0, 0.0, 0.0 ) );
        verts.push_back( Vector3( 0.0, 0.0, 0.0 ) );
        indices.push_back( 2 );
        indices.push_back( 1 );
        indices.push_back( 0 );
        // Note: Godot uses clockwise winding order for front faces of triangle primitive modes.
        normals.push_back( Vector3( 0, 0, 1 ) );
        normals.push_back( Vector3( 1, 0, 1 ) );
        normals.push_back( Vector3( 1, 0, 1 ) );
        uvs.push_back( Vector2( 0, 0 ) );
        uvs.push_back( Vector2( 0, 1 ) );
        uvs.push_back( Vector2( 1, 1 ) );

        Array surface_array;
        surface_array.resize( ArrayMesh::ARRAY_MAX );
        surface_array[ArrayMesh::ARRAY_VERTEX] = verts;
        surface_array[ArrayMesh::ARRAY_INDEX] = indices;
        surface_array[ArrayMesh::ARRAY_NORMAL] = normals;
        surface_array[ArrayMesh::ARRAY_TEX_UV] = uvs;

        Ref<ArrayMesh> mesh;
        mesh.instantiate();
        mesh->add_surface_from_arrays( ArrayMesh::PRIMITIVE_TRIANGLES, surface_array );
        Ref<StandardMaterial3D> material;
        material.instantiate();
        Ref<godot::ImageTexture> texture = createRandomTexture( 300, 300 );
        // const String path( "D:/Downloads/test.jpg" );
        // Ref<godot::ImageTexture> texture = loadTextureFromFile( path );
        material->set_texture( StandardMaterial3D::TextureParam::TEXTURE_ALBEDO, texture );
        material->set_texture_filter( BaseMaterial3D::TextureFilter::TEXTURE_FILTER_NEAREST );
        MeshInstance3D *meshInstance = memnew( MeshInstance3D );
        meshInstance->set_mesh( mesh );
        meshInstance->set_material_override( material );
        meshInstance->set_scale( Vector3( 300, 300, 300 ) );
        return meshInstance;
    }

    godot::MeshInstance3D *createCube()
    {
        using namespace godot;
        const Vector3 verticesData[8] = { Vector3( 0.0, 0.0, 0.0 ), Vector3( 1.0, 0.0, 0.0 ),
                                          Vector3( 1.0, 0.0, 1.0 ), Vector3( 0.0, 0.0, 1.0 ),
                                          Vector3( 0.0, 1.0, 0.0 ), Vector3( 1.0, 1.0, 0.0 ),
                                          Vector3( 1.0, 1.0, 1.0 ), Vector3( 0.0, 1.0, 1.0 ) };
        const int32_t indicesData[36] = { 0, 1, 2, 0, 2, 3, 4, 6, 5, 4, 7, 6, 0, 5, 1, 0, 4, 5,
                                          0, 7, 4, 0, 3, 7, 1, 5, 6, 1, 6, 2, 3, 2, 6, 3, 6, 7 };
        const Vector3 normalsData[6] = { Vector3( 0, -1, 0 ), Vector3( 0, 1, 0 ),
                                         Vector3( 0, 0, -1 ), Vector3( -1, 0, 0 ),
                                         Vector3( -1, 0, 0 ), Vector3( 1, 0, 0 ) };
        const Vector2 uv_data[8] = {
            Vector2( 0, 0 ), Vector2( 1, 0 ), Vector2( 1, 1 ), Vector2( 0, 1 ),
            Vector2( 0, 0 ), Vector2( 1, 0 ), Vector2( 1, 1 ), Vector2( 0, 1 ),
        };

        PackedVector3Array verts;
        PackedInt32Array indices;
        PackedVector3Array normals;
        PackedVector2Array uvs;
        for ( int32_t i = 0; i < 8; ++i )
        {
            verts.push_back( verticesData[i] );
        }
        for ( int32_t i = 35; i >= 0; --i )
        {
            indices.push_back( indicesData[i] );
        }
        for ( int32_t i = 0; i < 6; ++i )
        {
            normals.push_back( normalsData[i] );
            normals.push_back( normalsData[i] );
            normals.push_back( normalsData[i] );
        }
        for ( int32_t i = 0; i < 8; ++i )
        {
            uvs.push_back( uv_data[i] );
        }

        Array surface_array;
        surface_array.resize( ArrayMesh::ARRAY_MAX );
        surface_array[ArrayMesh::ARRAY_VERTEX] = verts;
        surface_array[ArrayMesh::ARRAY_INDEX] = indices;
        // surface_array[ArrayMesh::ARRAY_NORMAL] = normals;
        surface_array[ArrayMesh::ARRAY_TEX_UV] = uvs;

        Ref<ArrayMesh> mesh;
        mesh.instantiate();
        mesh->add_surface_from_arrays( ArrayMesh::PRIMITIVE_TRIANGLES, surface_array );
        Ref<StandardMaterial3D> material;
        material.instantiate();
        material->set_albedo( Color( 0.1f, 0.95f, 0.10f ) );
        Ref<godot::ImageTexture> texture = createRandomTexture( 10, 10 );
        // Ref<godot::ImageTexture> texture =
        //     loadTextureFromFile( "C:/Users/xuwze/Downloads/color_palette.png" );
        material->set_texture( StandardMaterial3D::TextureParam::TEXTURE_ALBEDO, texture );
        material->set_texture_filter( BaseMaterial3D::TextureFilter::TEXTURE_FILTER_NEAREST );
        material->set_cull_mode( BaseMaterial3D::CULL_BACK );
        MeshInstance3D *meshInstance = memnew( MeshInstance3D );
        meshInstance->set_mesh( mesh );
        meshInstance->set_material_overlay( material );
        meshInstance->set_scale( Vector3( 30, 30, 30 ) );
        Transform3D trans = meshInstance->get_transform();
        trans.origin = Vector3( 100, 50, 70 );
        meshInstance->set_transform( trans );
        return meshInstance;
    }

    godot::Ref<godot::ImageTexture> loadTextureFromFile( const godot::String &path )
    {
        using namespace godot;
        Ref<godot::Image> image = godot::Image::load_from_file( path );
        Ref<godot::ImageTexture> texture = godot::ImageTexture::create_from_image( image );
        return texture;
    }

    godot::Ref<godot::ImageTexture> createRandomTexture( int width, int height )
    {
        using namespace godot;
        Ref<godot::Image> image =
            godot::Image::create( width, height, false, godot::Image::FORMAT_RGBA8 );
        godot::RandomNumberGenerator rng;
        rng.set_seed( static_cast<uint32_t>( time( nullptr ) ) );
        for ( int y = 0; y < height; ++y )
        {
            for ( int x = 0; x < width; ++x )
            {
                // godot::Color color = godot::Color( 0.2549f, 0.9137f, 0.788f, 1.0 );
                godot::Color color = godot::Color( rng.randf(), rng.randf(), rng.randf(), 1.0 );
                image->set_pixel( x, y, color );
            }
        }
        Ref<godot::ImageTexture> texture = godot::ImageTexture::create_from_image( image );
        return texture;
    }

    godot::Vector3 test_editor_camera_direction;
    void testEditorCameraStates()
    {
        if ( godot::Engine::get_singleton()->is_editor_hint() )
        {

            godot::EditorInterface *editor_interface = godot::EditorInterface::get_singleton();
            godot::Node *editor_root_node = editor_interface->get_edited_scene_root();
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
                            godot::Transform3D camera_transform =
                                editor_camera->get_camera_transform();
                            godot::Vector3 direction = -camera_transform.basis.get_column( 2 );
                            if ( test_editor_camera_direction != direction )
                            {
                                test_editor_camera_direction = direction;
                                godot::UtilityFunctions::push_warning(
                                    "-------------------------------------------------------" );
                                godot::UtilityFunctions::push_warning(
                                    "-get_camera_transform basis.get_column( 2 )", direction );
                                godot::UtilityFunctions::push_warning(
                                    "-get_transform basis.get_column( 2 )",
                                    -editor_camera->get_transform().basis.get_column( 2 ) );
                                godot::UtilityFunctions::push_warning(
                                    "-get_global_transform basis[2]",
                                    -editor_camera->get_global_transform().basis.get_column( 2 ) );
                            }
                        }
                    }
                }
            }
        }
    }

    void createMatrixOfCubes( godot::Vector<godot::MeshInstance3D *> &instances )
    {
        using namespace godot;

        // 计算每个方块应该放置的位置，使得网格中心对齐到原点
        for ( int row = 0; row < GRID_SIZE; ++row )
        {
            for ( int col = 0; col < GRID_SIZE; ++col )
            {
                MeshInstance3D *instance = createCube();
                Transform3D trans;

                // 计算方块的位置，使得原点为网格中心
                // 注意：由于我们使用了SPACING作为方块之间的距离，包括方块自身的尺寸，
                // 所以每个方块的中心位置需要减去CUBE_SIZE/2和考虑网格中心偏移。
                float offsetX = ( GRID_SIZE * SPACING - CUBE_SIZE ) / 2.0f - col * SPACING;
                float offsetZ = ( GRID_SIZE * SPACING - CUBE_SIZE ) / 2.0f - row * SPACING;

                trans.origin.x = offsetX + CUBE_SIZE / 2.0f;
                trans.origin.z = offsetZ + CUBE_SIZE / 2.0f;
                instance->set_transform( trans );
                instance->set_scale( Vector3( CUBE_SIZE, CUBE_SIZE, CUBE_SIZE ) );
                instance->hide(); // 暂时隐藏，如果需要显示可以移除这行代码
                instances.push_back( instance );
            }
        }
    }

} // namespace CesiumForGodot
