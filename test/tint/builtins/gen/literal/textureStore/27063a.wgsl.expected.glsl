#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, rgba8) uniform highp writeonly image2DArray arg_0;
void textureStore_27063a() {
  imageStore(arg_0, ivec3(uvec3(uvec2(1u), uint(1))), vec4(1.0f));
}

void fragment_main() {
  textureStore_27063a();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

layout(binding = 0, rgba8) uniform highp writeonly image2DArray arg_0;
void textureStore_27063a() {
  imageStore(arg_0, ivec3(uvec3(uvec2(1u), uint(1))), vec4(1.0f));
}

void compute_main() {
  textureStore_27063a();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
