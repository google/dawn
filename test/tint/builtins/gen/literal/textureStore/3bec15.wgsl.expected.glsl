//
// fragment_main
//
#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, rgba8ui) uniform highp writeonly uimage2D f_arg_0;
void textureStore_3bec15() {
  imageStore(f_arg_0, ivec2(1, 0), uvec4(1u));
}
void main() {
  textureStore_3bec15();
}
//
// compute_main
//
#version 310 es

layout(binding = 0, rgba8ui) uniform highp writeonly uimage2D arg_0;
void textureStore_3bec15() {
  imageStore(arg_0, ivec2(1, 0), uvec4(1u));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  textureStore_3bec15();
}
