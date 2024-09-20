#version 460
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  vec4 tint_symbol;
} v;
layout(binding = 0, rg32f) uniform highp image3D arg_0;
vec4 textureLoad_126466() {
  ivec3 arg_1 = ivec3(1);
  vec4 res = imageLoad(arg_0, ivec3(arg_1));
  return res;
}
void main() {
  v.tint_symbol = textureLoad_126466();
}
#version 460

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  vec4 tint_symbol;
} v;
layout(binding = 0, rg32f) uniform highp image3D arg_0;
vec4 textureLoad_126466() {
  ivec3 arg_1 = ivec3(1);
  vec4 res = imageLoad(arg_0, ivec3(arg_1));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.tint_symbol = textureLoad_126466();
}
