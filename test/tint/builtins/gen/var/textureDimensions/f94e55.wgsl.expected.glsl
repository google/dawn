#version 460

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  uvec2 inner;
} prevent_dce;

layout(binding = 0, rg32f) uniform highp image2D arg_0;
uvec2 textureDimensions_f94e55() {
  uvec2 res = uvec2(imageSize(arg_0));
  return res;
}

void fragment_main() {
  prevent_dce.inner = textureDimensions_f94e55();
}

void main() {
  fragment_main();
  return;
}
#version 460

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  uvec2 inner;
} prevent_dce;

layout(binding = 0, rg32f) uniform highp image2D arg_0;
uvec2 textureDimensions_f94e55() {
  uvec2 res = uvec2(imageSize(arg_0));
  return res;
}

void compute_main() {
  prevent_dce.inner = textureDimensions_f94e55();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
