#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, rgba8) uniform highp writeonly image2D arg_0;
void textureStore_7f7fae() {
  imageStore(arg_0, ivec2(1, 0), vec4(1.0f));
}

void fragment_main() {
  textureStore_7f7fae();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

layout(binding = 0, rgba8) uniform highp writeonly image2D arg_0;
void textureStore_7f7fae() {
  imageStore(arg_0, ivec2(1, 0), vec4(1.0f));
}

void compute_main() {
  textureStore_7f7fae();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
