#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, rgba32f) uniform highp writeonly image2DArray arg_0;
void textureStore_5841f8() {
  uvec2 arg_1 = uvec2(1u);
  uint arg_2 = 1u;
  vec4 arg_3 = vec4(1.0f);
  imageStore(arg_0, ivec3(uvec3(arg_1, arg_2)), arg_3);
}

void fragment_main() {
  textureStore_5841f8();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

layout(binding = 0, rgba32f) uniform highp writeonly image2DArray arg_0;
void textureStore_5841f8() {
  uvec2 arg_1 = uvec2(1u);
  uint arg_2 = 1u;
  vec4 arg_3 = vec4(1.0f);
  imageStore(arg_0, ivec3(uvec3(arg_1, arg_2)), arg_3);
}

void compute_main() {
  textureStore_5841f8();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
