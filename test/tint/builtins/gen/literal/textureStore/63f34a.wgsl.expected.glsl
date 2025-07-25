//
// fragment_main
//
#version 460
precision highp float;
precision highp int;

layout(binding = 0, rg32i) uniform highp iimage2D f_arg_0;
void textureStore_63f34a() {
  imageStore(f_arg_0, ivec2(uvec2(1u)), ivec4(1));
}
void main() {
  textureStore_63f34a();
}
//
// compute_main
//
#version 460

layout(binding = 0, rg32i) uniform highp iimage2D arg_0;
void textureStore_63f34a() {
  imageStore(arg_0, ivec2(uvec2(1u)), ivec4(1));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  textureStore_63f34a();
}
