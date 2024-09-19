#version 460

layout(binding = 0, r8) uniform highp image2DArray arg_0;
void textureStore_6f0c92() {
  imageStore(arg_0, ivec3(uvec3(uvec2(1u), uint(1))), vec4(1.0f));
}

void fragment_main() {
  textureStore_6f0c92();
}

void main() {
  fragment_main();
  return;
}
#version 460

layout(binding = 0, r8) uniform highp image2DArray arg_0;
void textureStore_6f0c92() {
  imageStore(arg_0, ivec3(uvec3(uvec2(1u), uint(1))), vec4(1.0f));
}

void compute_main() {
  textureStore_6f0c92();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
