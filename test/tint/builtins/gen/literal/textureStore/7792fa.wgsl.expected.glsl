#version 460

layout(binding = 0, rg32i) uniform highp iimage3D arg_0;
void textureStore_7792fa() {
  imageStore(arg_0, ivec3(uvec3(1u)), ivec4(1));
}

void fragment_main() {
  textureStore_7792fa();
}

void main() {
  fragment_main();
  return;
}
#version 460

layout(binding = 0, rg32i) uniform highp iimage3D arg_0;
void textureStore_7792fa() {
  imageStore(arg_0, ivec3(uvec3(1u)), ivec4(1));
}

void compute_main() {
  textureStore_7792fa();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
