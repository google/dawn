#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, r32i) uniform highp iimage2D arg_0;
void textureStore_bcc97a() {
  ivec2 arg_1 = ivec2(1);
  ivec4 arg_2 = ivec4(1);
  imageStore(arg_0, arg_1, arg_2);
}

void fragment_main() {
  textureStore_bcc97a();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

layout(binding = 0, r32i) uniform highp iimage2D arg_0;
void textureStore_bcc97a() {
  ivec2 arg_1 = ivec2(1);
  ivec4 arg_2 = ivec4(1);
  imageStore(arg_0, arg_1, arg_2);
}

void compute_main() {
  textureStore_bcc97a();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
