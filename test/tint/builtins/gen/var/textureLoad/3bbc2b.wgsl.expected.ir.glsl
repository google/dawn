#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  vec4 tint_symbol;
} v;
layout(binding = 0, r32f) uniform highp image2D arg_0;
vec4 textureLoad_3bbc2b() {
  int arg_1 = 1;
  vec4 res = imageLoad(arg_0, ivec2(ivec2(arg_1, 0)));
  return res;
}
void main() {
  v.tint_symbol = textureLoad_3bbc2b();
}
#version 310 es

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  vec4 tint_symbol;
} v;
layout(binding = 0, r32f) uniform highp image2D arg_0;
vec4 textureLoad_3bbc2b() {
  int arg_1 = 1;
  vec4 res = imageLoad(arg_0, ivec2(ivec2(arg_1, 0)));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.tint_symbol = textureLoad_3bbc2b();
}
