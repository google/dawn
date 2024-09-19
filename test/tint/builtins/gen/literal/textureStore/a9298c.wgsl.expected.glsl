#version 460

layout(binding = 0, rg32ui) uniform highp uimage2D arg_0;
void textureStore_a9298c() {
  imageStore(arg_0, ivec2(uvec2(1u, 0u)), uvec4(1u));
}

void fragment_main() {
  textureStore_a9298c();
}

void main() {
  fragment_main();
  return;
}
#version 460

layout(binding = 0, rg32ui) uniform highp uimage2D arg_0;
void textureStore_a9298c() {
  imageStore(arg_0, ivec2(uvec2(1u, 0u)), uvec4(1u));
}

void compute_main() {
  textureStore_a9298c();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
