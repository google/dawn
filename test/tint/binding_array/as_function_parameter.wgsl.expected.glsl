#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std140)
uniform f_TintTextureUniformData_ubo {
  uvec4 metadata[1];
} v;
uniform highp sampler2D f_sampled_textures[4];
void do_texture_load() {
  uint v_1 = (0u + uint(0));
  uint v_2 = (v.metadata[(v_1 / 4u)][(v_1 % 4u)] - 1u);
  uint v_3 = min(uint(0), v_2);
  uvec2 v_4 = (uvec2(textureSize(f_sampled_textures[0], int(v_3))) - uvec2(1u));
  ivec2 v_5 = ivec2(min(uvec2(ivec2(0)), v_4));
  vec4 texture_load = texelFetch(f_sampled_textures[0], v_5, int(v_3));
}
void main() {
  do_texture_load();
}
