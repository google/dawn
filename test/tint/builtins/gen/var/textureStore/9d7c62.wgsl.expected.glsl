#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, rgba8ui) uniform highp writeonly uimage2D arg_0;
void textureStore_9d7c62() {
  int arg_1 = 1;
  uvec4 arg_2 = uvec4(1u);
  imageStore(arg_0, ivec2(arg_1, 0), arg_2);
}

void fragment_main() {
  textureStore_9d7c62();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

layout(binding = 0, rgba8ui) uniform highp writeonly uimage2D arg_0;
void textureStore_9d7c62() {
  int arg_1 = 1;
  uvec4 arg_2 = uvec4(1u);
  imageStore(arg_0, ivec2(arg_1, 0), arg_2);
}

void compute_main() {
  textureStore_9d7c62();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
