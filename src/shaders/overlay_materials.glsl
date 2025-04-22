// Copyright © 2025 David Xu

R"(
shader_type spatial;
// render_mode blend_mix, depth_test_disable; 

uniform sampler2D _overlay_texture_0;
uniform sampler2D _overlay_texture_1;
uniform sampler2D _overlay_texture_2;
uniform sampler2D _overlay_texture_Clipping;

uniform sampler2D _overlay_texture_coordinate_index_0;
uniform sampler2D _overlay_texture_coordinate_index_1;
uniform sampler2D _overlay_texture_coordinate_index_2;
uniform sampler2D _overlay_texture_coordinate_index_Clipping;

uniform vec4 _overlay_translation_and_scale_0 = vec4(0.0, 0.0, 1.0, 1.0);
uniform vec4 _overlay_translation_and_scale_1 = vec4(0.0, 0.0, 1.0, 1.0);
uniform vec4 _overlay_translation_and_scale_2 = vec4(0.0, 0.0, 1.0, 1.0);
uniform vec4 _overlay_translation_and_scale_Clipping = vec4(0.0, 0.0, 1.0, 1.0);



void fragment() {
    // vec2 custom_uv = UV * overlay_translation_and_scale_0.zw + vec2(overlay_translation_and_scale_0.x, overlay_translation_and_scale_0.y);
    // vec4 tex_color = texture(overlay_texture_0, custom_uv);
    // ALBEDO = tex_color;
    ALBEDO = vec3(0.4, 0.6, 0.9);
}

)"
