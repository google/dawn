//
// fragment_main
//
#version 460
precision highp float;
precision highp int;

layout(binding = 0, rg32ui) uniform highp uimage2D f_arg_0;
void textureStore_4cce74() {
  imageStore(f_arg_0, ivec2(1, 0), uvec4(1u));
}
void main() {
  textureStore_4cce74();
}
//
// compute_main
//
#version 460

layout(binding = 0, rg32ui) uniform highp uimage2D arg_0;
void textureStore_4cce74() {
  imageStore(arg_0, ivec2(1, 0), uvec4(1u));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  textureStore_4cce74();
}
