#version 310 es

layout(rgba32ui) uniform highp writeonly uimage2DArray arg_0;
void textureStore_8e0479() {
  imageStore(arg_0, ivec3(0, 0, 1), uvec4(0u, 0u, 0u, 0u));
}

vec4 vertex_main() {
  textureStore_8e0479();
  return vec4(0.0f, 0.0f, 0.0f, 0.0f);
}

void main() {
  vec4 inner_result = vertex_main();
  gl_Position = inner_result;
  gl_Position.y = -(gl_Position.y);
  gl_Position.z = ((2.0f * gl_Position.z) - gl_Position.w);
  return;
}
#version 310 es
precision mediump float;

layout(rgba32ui) uniform highp writeonly uimage2DArray arg_0;
void textureStore_8e0479() {
  imageStore(arg_0, ivec3(0, 0, 1), uvec4(0u, 0u, 0u, 0u));
}

void fragment_main() {
  textureStore_8e0479();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

layout(rgba32ui) uniform highp writeonly uimage2DArray arg_0;
void textureStore_8e0479() {
  imageStore(arg_0, ivec3(0, 0, 1), uvec4(0u, 0u, 0u, 0u));
}

void compute_main() {
  textureStore_8e0479();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
