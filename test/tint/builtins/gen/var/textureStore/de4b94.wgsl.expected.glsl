#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, r32i) uniform highp writeonly iimage2D arg_0;
void textureStore_de4b94() {
  uint arg_1 = 1u;
  ivec4 arg_2 = ivec4(1);
  imageStore(arg_0, ivec2(uvec2(arg_1, 0u)), arg_2);
}

void fragment_main() {
  textureStore_de4b94();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

layout(binding = 0, r32i) uniform highp writeonly iimage2D arg_0;
void textureStore_de4b94() {
  uint arg_1 = 1u;
  ivec4 arg_2 = ivec4(1);
  imageStore(arg_0, ivec2(uvec2(arg_1, 0u)), arg_2);
}

void compute_main() {
  textureStore_de4b94();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
