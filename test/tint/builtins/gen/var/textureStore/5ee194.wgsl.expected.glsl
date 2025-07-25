//
// fragment_main
//
#version 460
precision highp float;
precision highp int;

layout(binding = 0, rg32f) uniform highp image2DArray f_arg_0;
void textureStore_5ee194() {
  uvec2 arg_1 = uvec2(1u);
  uint arg_2 = 1u;
  vec4 arg_3 = vec4(1.0f);
  uint v = arg_2;
  vec4 v_1 = arg_3;
  ivec2 v_2 = ivec2(arg_1);
  imageStore(f_arg_0, ivec3(v_2, int(v)), v_1);
}
void main() {
  textureStore_5ee194();
}
//
// compute_main
//
#version 460

layout(binding = 0, rg32f) uniform highp image2DArray arg_0;
void textureStore_5ee194() {
  uvec2 arg_1 = uvec2(1u);
  uint arg_2 = 1u;
  vec4 arg_3 = vec4(1.0f);
  uint v = arg_2;
  vec4 v_1 = arg_3;
  ivec2 v_2 = ivec2(arg_1);
  imageStore(arg_0, ivec3(v_2, int(v)), v_1);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  textureStore_5ee194();
}
