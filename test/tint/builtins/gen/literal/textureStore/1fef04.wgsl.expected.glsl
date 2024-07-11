#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, r32i) uniform highp iimage2D arg_0;
void textureStore_1fef04() {
  imageStore(arg_0, ivec2(uvec2(1u, 0u)), ivec4(1));
}

void fragment_main() {
  textureStore_1fef04();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

layout(binding = 0, r32i) uniform highp iimage2D arg_0;
void textureStore_1fef04() {
  imageStore(arg_0, ivec2(uvec2(1u, 0u)), ivec4(1));
}

void compute_main() {
  textureStore_1fef04();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
