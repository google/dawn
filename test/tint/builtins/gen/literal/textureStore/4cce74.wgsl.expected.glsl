#version 460

layout(binding = 0, rg32ui) uniform highp uimage2D arg_0;
void textureStore_4cce74() {
  imageStore(arg_0, ivec2(1, 0), uvec4(1u));
}

void fragment_main() {
  textureStore_4cce74();
}

void main() {
  fragment_main();
  return;
}
#version 460

layout(binding = 0, rg32ui) uniform highp uimage2D arg_0;
void textureStore_4cce74() {
  imageStore(arg_0, ivec2(1, 0), uvec4(1u));
}

void compute_main() {
  textureStore_4cce74();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
