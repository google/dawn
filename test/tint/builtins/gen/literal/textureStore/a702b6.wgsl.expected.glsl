#version 460

layout(binding = 0, rg32ui) uniform highp uimage2DArray arg_0;
void textureStore_a702b6() {
  imageStore(arg_0, ivec3(ivec2(1), 1), uvec4(1u));
}

void fragment_main() {
  textureStore_a702b6();
}

void main() {
  fragment_main();
  return;
}
#version 460

layout(binding = 0, rg32ui) uniform highp uimage2DArray arg_0;
void textureStore_a702b6() {
  imageStore(arg_0, ivec3(ivec2(1), 1), uvec4(1u));
}

void compute_main() {
  textureStore_a702b6();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
