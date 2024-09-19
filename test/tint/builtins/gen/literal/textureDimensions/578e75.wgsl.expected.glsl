#version 460

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  uvec2 inner;
} prevent_dce;

layout(binding = 0, r8) uniform highp image2DArray arg_0;
uvec2 textureDimensions_578e75() {
  uvec2 res = uvec2(imageSize(arg_0).xy);
  return res;
}

void fragment_main() {
  prevent_dce.inner = textureDimensions_578e75();
}

void main() {
  fragment_main();
  return;
}
#version 460

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  uvec2 inner;
} prevent_dce;

layout(binding = 0, r8) uniform highp image2DArray arg_0;
uvec2 textureDimensions_578e75() {
  uvec2 res = uvec2(imageSize(arg_0).xy);
  return res;
}

void compute_main() {
  prevent_dce.inner = textureDimensions_578e75();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
