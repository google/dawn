#version 310 es

layout(binding = 0, std140)
uniform level_block_1_ubo {
  uint inner;
} v;
layout(binding = 1, std140)
uniform coords_block_1_ubo {
  uvec2 inner;
} v_1;
layout(binding = 0, std140)
uniform TintTextureUniformData_1_ubo {
  uvec4 metadata[1];
} v_2;
uniform highp sampler2D tex;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uvec2 v_3 = v_1.inner;
  uint v_4 = min(v.inner, (v_2.metadata[(0u / 4u)][(0u % 4u)] - 1u));
  ivec2 v_5 = ivec2(min(v_3, (uvec2(textureSize(tex, int(v_4))) - uvec2(1u))));
  float res = texelFetch(tex, v_5, int(v_4)).x;
}
