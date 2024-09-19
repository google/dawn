#version 460

layout(binding = 0, r8) uniform highp image2D arg_0;
void textureStore_65ba8b() {
  imageStore(arg_0, ivec2(1), vec4(1.0f));
}

void fragment_main() {
  textureStore_65ba8b();
}

void main() {
  fragment_main();
  return;
}
#version 460

layout(binding = 0, r8) uniform highp image2D arg_0;
void textureStore_65ba8b() {
  imageStore(arg_0, ivec2(1), vec4(1.0f));
}

void compute_main() {
  textureStore_65ba8b();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
