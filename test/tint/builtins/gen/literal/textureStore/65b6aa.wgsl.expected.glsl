#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, rgba8i) uniform highp writeonly iimage2D arg_0;
void textureStore_65b6aa() {
  imageStore(arg_0, ivec2(1, 0), ivec4(1));
}

void fragment_main() {
  textureStore_65b6aa();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

layout(binding = 0, rgba8i) uniform highp writeonly iimage2D arg_0;
void textureStore_65b6aa() {
  imageStore(arg_0, ivec2(1, 0), ivec4(1));
}

void compute_main() {
  textureStore_65b6aa();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
