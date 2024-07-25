#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  uvec3 inner;
} prevent_dce;

layout(binding = 0, rgba8i) uniform highp writeonly iimage3D arg_0;
uvec3 textureDimensions_7ea4b5() {
  uvec3 res = uvec3(imageSize(arg_0));
  return res;
}

void fragment_main() {
  prevent_dce.inner = textureDimensions_7ea4b5();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  uvec3 inner;
} prevent_dce;

layout(binding = 0, rgba8i) uniform highp writeonly iimage3D arg_0;
uvec3 textureDimensions_7ea4b5() {
  uvec3 res = uvec3(imageSize(arg_0));
  return res;
}

void compute_main() {
  prevent_dce.inner = textureDimensions_7ea4b5();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
