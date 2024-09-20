#version 460
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  vec4 tint_symbol;
} v;
layout(binding = 0, r8) uniform highp image2DArray arg_0;
vec4 textureLoad_bc882d() {
  uvec2 arg_1 = uvec2(1u);
  int arg_2 = 1;
  int v_1 = arg_2;
  ivec2 v_2 = ivec2(arg_1);
  vec4 res = imageLoad(arg_0, ivec3(v_2, int(v_1)));
  return res;
}
void main() {
  v.tint_symbol = textureLoad_bc882d();
}
#version 460

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  vec4 tint_symbol;
} v;
layout(binding = 0, r8) uniform highp image2DArray arg_0;
vec4 textureLoad_bc882d() {
  uvec2 arg_1 = uvec2(1u);
  int arg_2 = 1;
  int v_1 = arg_2;
  ivec2 v_2 = ivec2(arg_1);
  vec4 res = imageLoad(arg_0, ivec3(v_2, int(v_1)));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.tint_symbol = textureLoad_bc882d();
}
