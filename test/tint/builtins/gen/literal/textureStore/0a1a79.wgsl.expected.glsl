#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, rgba8ui) uniform highp writeonly uimage2DArray arg_0;
void textureStore_0a1a79() {
  imageStore(arg_0, ivec3(uvec3(uvec2(1u), uint(1))), uvec4(1u));
}

void fragment_main() {
  textureStore_0a1a79();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

layout(binding = 0, rgba8ui) uniform highp writeonly uimage2DArray arg_0;
void textureStore_0a1a79() {
  imageStore(arg_0, ivec3(uvec3(uvec2(1u), uint(1))), uvec4(1u));
}

void compute_main() {
  textureStore_0a1a79();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
