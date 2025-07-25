//
// fragment_main
//
#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, rgba8) uniform highp writeonly image2D f_arg_0;
void textureStore_e0b666() {
  int arg_1 = 1;
  vec4 arg_2 = vec4(1.0f);
  vec4 v = arg_2.zyxw;
  imageStore(f_arg_0, ivec2(arg_1, 0), v);
}
void main() {
  textureStore_e0b666();
}
//
// compute_main
//
#version 310 es

layout(binding = 0, rgba8) uniform highp writeonly image2D arg_0;
void textureStore_e0b666() {
  int arg_1 = 1;
  vec4 arg_2 = vec4(1.0f);
  vec4 v = arg_2.zyxw;
  imageStore(arg_0, ivec2(arg_1, 0), v);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  textureStore_e0b666();
}
