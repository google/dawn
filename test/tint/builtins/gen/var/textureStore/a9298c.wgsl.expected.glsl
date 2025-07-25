//
// fragment_main
//
#version 460
precision highp float;
precision highp int;

layout(binding = 0, rg32ui) uniform highp uimage2D f_arg_0;
void textureStore_a9298c() {
  uint arg_1 = 1u;
  uvec4 arg_2 = uvec4(1u);
  uvec4 v = arg_2;
  imageStore(f_arg_0, ivec2(uvec2(arg_1, 0u)), v);
}
void main() {
  textureStore_a9298c();
}
//
// compute_main
//
#version 460

layout(binding = 0, rg32ui) uniform highp uimage2D arg_0;
void textureStore_a9298c() {
  uint arg_1 = 1u;
  uvec4 arg_2 = uvec4(1u);
  uvec4 v = arg_2;
  imageStore(arg_0, ivec2(uvec2(arg_1, 0u)), v);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  textureStore_a9298c();
}
