#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, rgba16i) uniform highp writeonly iimage2D arg_0;
void textureStore_b76ff3() {
  imageStore(arg_0, ivec2(uvec2(1u)), ivec4(1));
}

void fragment_main() {
  textureStore_b76ff3();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

layout(binding = 0, rgba16i) uniform highp writeonly iimage2D arg_0;
void textureStore_b76ff3() {
  imageStore(arg_0, ivec2(uvec2(1u)), ivec4(1));
}

void compute_main() {
  textureStore_b76ff3();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
