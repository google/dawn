#version 460

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  vec4 inner;
} prevent_dce;

layout(binding = 0, rg32f) uniform highp image2D arg_0;
vec4 textureLoad_b4d6c4() {
  vec4 res = imageLoad(arg_0, ivec2(uvec2(1u)));
  return res;
}

void fragment_main() {
  prevent_dce.inner = textureLoad_b4d6c4();
}

void main() {
  fragment_main();
  return;
}
#version 460

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  vec4 inner;
} prevent_dce;

layout(binding = 0, rg32f) uniform highp image2D arg_0;
vec4 textureLoad_b4d6c4() {
  vec4 res = imageLoad(arg_0, ivec2(uvec2(1u)));
  return res;
}

void compute_main() {
  prevent_dce.inner = textureLoad_b4d6c4();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
