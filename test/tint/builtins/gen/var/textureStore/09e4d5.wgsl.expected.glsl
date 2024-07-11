#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, rgba8) uniform highp writeonly image2D arg_0;
void textureStore_09e4d5() {
  uint arg_1 = 1u;
  vec4 arg_2 = vec4(1.0f);
  imageStore(arg_0, ivec2(uvec2(arg_1, 0u)), arg_2.bgra);
}

void fragment_main() {
  textureStore_09e4d5();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

layout(binding = 0, rgba8) uniform highp writeonly image2D arg_0;
void textureStore_09e4d5() {
  uint arg_1 = 1u;
  vec4 arg_2 = vec4(1.0f);
  imageStore(arg_0, ivec2(uvec2(arg_1, 0u)), arg_2.bgra);
}

void compute_main() {
  textureStore_09e4d5();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
