#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  uvec4 tint_symbol;
} v;
layout(binding = 0, r32ui) uniform highp uimage2D arg_0;
uvec4 textureLoad_02ef1f() {
  uvec4 res = imageLoad(arg_0, ivec2(ivec2(1)));
  return res;
}
void main() {
  v.tint_symbol = textureLoad_02ef1f();
}
#version 310 es

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  uvec4 tint_symbol;
} v;
layout(binding = 0, r32ui) uniform highp uimage2D arg_0;
uvec4 textureLoad_02ef1f() {
  uvec4 res = imageLoad(arg_0, ivec2(ivec2(1)));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.tint_symbol = textureLoad_02ef1f();
}
