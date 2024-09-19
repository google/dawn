#version 460

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  vec4 inner;
} prevent_dce;

layout(binding = 0, rg32f) uniform highp image2D arg_0;
vec4 textureLoad_defd9a() {
  vec4 res = imageLoad(arg_0, ivec2(1));
  return res;
}

void fragment_main() {
  prevent_dce.inner = textureLoad_defd9a();
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
vec4 textureLoad_defd9a() {
  vec4 res = imageLoad(arg_0, ivec2(1));
  return res;
}

void compute_main() {
  prevent_dce.inner = textureLoad_defd9a();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
