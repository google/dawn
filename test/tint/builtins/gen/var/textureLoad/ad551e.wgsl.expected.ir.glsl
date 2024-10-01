#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  uvec4 tint_symbol;
} v;
layout(binding = 0, r32ui) uniform highp uimage2D arg_0;
uvec4 textureLoad_ad551e() {
  uint arg_1 = 1u;
  uvec4 res = imageLoad(arg_0, ivec2(uvec2(arg_1, 0u)));
  return res;
}
void main() {
  v.tint_symbol = textureLoad_ad551e();
}
#version 310 es

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  uvec4 tint_symbol;
} v;
layout(binding = 0, r32ui) uniform highp uimage2D arg_0;
uvec4 textureLoad_ad551e() {
  uint arg_1 = 1u;
  uvec4 res = imageLoad(arg_0, ivec2(uvec2(arg_1, 0u)));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.tint_symbol = textureLoad_ad551e();
}
