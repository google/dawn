#version 460

layout(binding = 0, rg32ui) uniform highp writeonly uimage2DArray arg_0;
void textureStore_dffb13() {
  ivec2 arg_1 = ivec2(1);
  uint arg_2 = 1u;
  uvec4 arg_3 = uvec4(1u);
  imageStore(arg_0, ivec3(arg_1, int(arg_2)), arg_3);
}

void fragment_main() {
  textureStore_dffb13();
}

void main() {
  fragment_main();
  return;
}
#version 460

layout(binding = 0, rg32ui) uniform highp writeonly uimage2DArray arg_0;
void textureStore_dffb13() {
  ivec2 arg_1 = ivec2(1);
  uint arg_2 = 1u;
  uvec4 arg_3 = uvec4(1u);
  imageStore(arg_0, ivec3(arg_1, int(arg_2)), arg_3);
}

void compute_main() {
  textureStore_dffb13();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
