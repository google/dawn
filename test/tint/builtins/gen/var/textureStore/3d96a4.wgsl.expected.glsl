#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, rgba32i) uniform highp writeonly iimage3D arg_0;
void textureStore_3d96a4() {
  uvec3 arg_1 = uvec3(1u);
  ivec4 arg_2 = ivec4(1);
  imageStore(arg_0, ivec3(arg_1), arg_2);
}

void fragment_main() {
  textureStore_3d96a4();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

layout(binding = 0, rgba32i) uniform highp writeonly iimage3D arg_0;
void textureStore_3d96a4() {
  uvec3 arg_1 = uvec3(1u);
  ivec4 arg_2 = ivec4(1);
  imageStore(arg_0, ivec3(arg_1), arg_2);
}

void compute_main() {
  textureStore_3d96a4();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
