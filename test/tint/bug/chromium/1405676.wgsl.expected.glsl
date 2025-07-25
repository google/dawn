#version 310 es

layout(binding = 0, std140)
uniform TintTextureUniformData_1_ubo {
  uvec4 metadata[1];
} v;
uniform highp isampler2D arg_0;
void d() {
  uint v_1 = (v.metadata[(0u / 4u)][(0u % 4u)] - 1u);
  uint v_2 = min(uint(0), v_1);
  uint v_3 = (uvec2(textureSize(arg_0, int(v_2))).x - 1u);
  ivec2 v_4 = ivec2(uvec2(min(uint(1), v_3), 0u));
  texelFetch(arg_0, v_4, int(v_2));
  float l = 0.14112000167369842529f;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
