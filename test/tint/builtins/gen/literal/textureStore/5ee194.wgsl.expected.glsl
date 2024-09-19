#version 460

layout(binding = 0, rg32f) uniform highp image2DArray arg_0;
void textureStore_5ee194() {
  imageStore(arg_0, ivec3(uvec3(uvec2(1u), 1u)), vec4(1.0f));
}

void fragment_main() {
  textureStore_5ee194();
}

void main() {
  fragment_main();
  return;
}
#version 460

layout(binding = 0, rg32f) uniform highp image2DArray arg_0;
void textureStore_5ee194() {
  imageStore(arg_0, ivec3(uvec3(uvec2(1u), 1u)), vec4(1.0f));
}

void compute_main() {
  textureStore_5ee194();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
