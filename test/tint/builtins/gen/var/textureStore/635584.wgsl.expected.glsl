#version 460

layout(binding = 0, rg32f) uniform highp image2D arg_0;
void textureStore_635584() {
  uint arg_1 = 1u;
  vec4 arg_2 = vec4(1.0f);
  imageStore(arg_0, ivec2(uvec2(arg_1, 0u)), arg_2);
}

void fragment_main() {
  textureStore_635584();
}

void main() {
  fragment_main();
  return;
}
#version 460

layout(binding = 0, rg32f) uniform highp image2D arg_0;
void textureStore_635584() {
  uint arg_1 = 1u;
  vec4 arg_2 = vec4(1.0f);
  imageStore(arg_0, ivec2(uvec2(arg_1, 0u)), arg_2);
}

void compute_main() {
  textureStore_635584();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
