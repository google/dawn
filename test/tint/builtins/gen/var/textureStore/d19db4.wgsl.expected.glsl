//
// fragment_main
//
#version 460
precision highp float;
precision highp int;

layout(binding = 0, rg32i) uniform highp iimage3D f_arg_0;
void textureStore_d19db4() {
  ivec3 arg_1 = ivec3(1);
  ivec4 arg_2 = ivec4(1);
  imageStore(f_arg_0, arg_1, arg_2);
}
void main() {
  textureStore_d19db4();
}
//
// compute_main
//
#version 460

layout(binding = 0, rg32i) uniform highp iimage3D arg_0;
void textureStore_d19db4() {
  ivec3 arg_1 = ivec3(1);
  ivec4 arg_2 = ivec4(1);
  imageStore(arg_0, arg_1, arg_2);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  textureStore_d19db4();
}
