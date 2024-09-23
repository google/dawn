#version 460

layout(binding = 0, rg32i) uniform highp iimage2D arg_0;
void textureStore_63f34a() {
  imageStore(arg_0, ivec2(uvec2(1u)), ivec4(1));
}

void fragment_main() {
  textureStore_63f34a();
}

void main() {
  fragment_main();
  return;
}
#version 460

layout(binding = 0, rg32i) uniform highp iimage2D arg_0;
void textureStore_63f34a() {
  imageStore(arg_0, ivec2(uvec2(1u)), ivec4(1));
}

void compute_main() {
  textureStore_63f34a();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
