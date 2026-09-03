#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std140)
uniform f_TintTextureUniformData_ubo {
  uvec4 metadata[1];
} v;
uniform highp sampler2D f_sampled_textures[4];
void main() {
  uint v_1 = min(0u, (v.metadata[0u].x - 1u));
  ivec2 v_2 = ivec2(min(uvec2(0u), (uvec2(textureSize(f_sampled_textures[0], int(v_1))) - uvec2(1u))));
  vec4 texture_load = texelFetch(f_sampled_textures[0], v_2, int(v_1));
}
