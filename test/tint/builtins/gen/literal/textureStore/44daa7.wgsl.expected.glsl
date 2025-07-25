//
// fragment_main
//
#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, rgba16f) uniform highp writeonly image2DArray f_arg_0;
void textureStore_44daa7() {
  imageStore(f_arg_0, ivec3(ivec2(1), int(1u)), vec4(1.0f));
}
void main() {
  textureStore_44daa7();
}
//
// compute_main
//
#version 310 es

layout(binding = 0, rgba16f) uniform highp writeonly image2DArray arg_0;
void textureStore_44daa7() {
  imageStore(arg_0, ivec3(ivec2(1), int(1u)), vec4(1.0f));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  textureStore_44daa7();
}
