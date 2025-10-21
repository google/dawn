#version 310 es

layout(binding = 0, std140)
uniform level_block_1_ubo {
  uvec4 inner[1];
} v;
layout(binding = 1, std140)
uniform coords_block_1_ubo {
  uvec4 inner[1];
} v_1;
layout(binding = 0, std140)
uniform TintTextureUniformData_1_ubo {
  uvec4 metadata[1];
} v_2;
uniform highp sampler2D tex;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uvec2 v_3 = v_1.inner[0u].xy;
  uvec4 v_4 = v.inner[0u];
  uint v_5 = min(v_4.x, (v_2.metadata[(0u / 4u)][(0u % 4u)] - 1u));
  ivec2 v_6 = ivec2(min(v_3, (uvec2(textureSize(tex, int(v_5))) - uvec2(1u))));
  float res = texelFetch(tex, v_6, int(v_5)).x;
}
