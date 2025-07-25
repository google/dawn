//
// fragment_main
//
#version 460
precision highp float;
precision highp int;

layout(binding = 0, r8) uniform highp writeonly image2D f_arg_0;
void textureStore_9e5bc2() {
  uvec2 arg_1 = uvec2(1u);
  vec4 arg_2 = vec4(1.0f);
  vec4 v = arg_2;
  imageStore(f_arg_0, ivec2(arg_1), v);
}
void main() {
  textureStore_9e5bc2();
}
//
// compute_main
//
#version 460

layout(binding = 0, r8) uniform highp writeonly image2D arg_0;
void textureStore_9e5bc2() {
  uvec2 arg_1 = uvec2(1u);
  vec4 arg_2 = vec4(1.0f);
  vec4 v = arg_2;
  imageStore(arg_0, ivec2(arg_1), v);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  textureStore_9e5bc2();
}
