#version 460

layout(binding = 0, rg32ui) uniform highp writeonly uimage2DArray arg_0;
void textureStore_4d359d() {
  imageStore(arg_0, ivec3(uvec3(uvec2(1u), uint(1))), uvec4(1u));
}

void fragment_main() {
  textureStore_4d359d();
}

void main() {
  fragment_main();
  return;
}
#version 460

layout(binding = 0, rg32ui) uniform highp writeonly uimage2DArray arg_0;
void textureStore_4d359d() {
  imageStore(arg_0, ivec3(uvec3(uvec2(1u), uint(1))), uvec4(1u));
}

void compute_main() {
  textureStore_4d359d();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
