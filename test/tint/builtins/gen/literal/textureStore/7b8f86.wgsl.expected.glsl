#version 460

layout(binding = 0, rg32f) uniform highp image3D arg_0;
void textureStore_7b8f86() {
  imageStore(arg_0, ivec3(uvec3(1u)), vec4(1.0f));
}

void fragment_main() {
  textureStore_7b8f86();
}

void main() {
  fragment_main();
  return;
}
#version 460

layout(binding = 0, rg32f) uniform highp image3D arg_0;
void textureStore_7b8f86() {
  imageStore(arg_0, ivec3(uvec3(1u)), vec4(1.0f));
}

void compute_main() {
  textureStore_7b8f86();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
