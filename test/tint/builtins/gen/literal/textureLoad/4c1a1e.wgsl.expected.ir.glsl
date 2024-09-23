#version 460
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  uvec4 tint_symbol;
} v;
layout(binding = 0, rg32ui) uniform highp uimage3D arg_0;
uvec4 textureLoad_4c1a1e() {
  uvec4 res = imageLoad(arg_0, ivec3(ivec3(1)));
  return res;
}
void main() {
  v.tint_symbol = textureLoad_4c1a1e();
}
#version 460

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  uvec4 tint_symbol;
} v;
layout(binding = 0, rg32ui) uniform highp uimage3D arg_0;
uvec4 textureLoad_4c1a1e() {
  uvec4 res = imageLoad(arg_0, ivec3(ivec3(1)));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.tint_symbol = textureLoad_4c1a1e();
}
