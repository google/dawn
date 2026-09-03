#version 310 es

layout(binding = 0, std140)
uniform TintTextureUniformData_1_ubo {
  uvec4 metadata[1];
} v;
uniform highp isampler2D arg_0;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uint v_1 = min(0u, (v.metadata[0u].x - 1u));
  ivec2 v_2 = ivec2(uvec2(min(1u, (uvec2(textureSize(arg_0, int(v_1))).x - 1u)), 0u));
  texelFetch(arg_0, v_2, int(v_1));
  float l = 0.14112000167369842529f;
}
