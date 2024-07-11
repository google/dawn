#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, rgba8) uniform highp writeonly image2D arg_0;
void textureStore_e46fd8() {
  imageStore(arg_0, ivec2(uvec2(1u)), vec4(1.0f));
}

void fragment_main() {
  textureStore_e46fd8();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

layout(binding = 0, rgba8) uniform highp writeonly image2D arg_0;
void textureStore_e46fd8() {
  imageStore(arg_0, ivec2(uvec2(1u)), vec4(1.0f));
}

void compute_main() {
  textureStore_e46fd8();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
