#version 310 es

layout(rgba16ui) uniform highp writeonly uimage2D arg_0;
void textureStore_b70ded() {
  uint arg_1 = 1u;
  uvec4 arg_2 = uvec4(1u);
  imageStore(arg_0, ivec2(uvec2(arg_1, 0u)), arg_2);
}

vec4 vertex_main() {
  textureStore_b70ded();
  return vec4(0.0f);
}

void main() {
  gl_PointSize = 1.0;
  vec4 inner_result = vertex_main();
  gl_Position = inner_result;
  gl_Position.y = -(gl_Position.y);
  gl_Position.z = ((2.0f * gl_Position.z) - gl_Position.w);
  return;
}
#version 310 es
precision highp float;

layout(rgba16ui) uniform highp writeonly uimage2D arg_0;
void textureStore_b70ded() {
  uint arg_1 = 1u;
  uvec4 arg_2 = uvec4(1u);
  imageStore(arg_0, ivec2(uvec2(arg_1, 0u)), arg_2);
}

void fragment_main() {
  textureStore_b70ded();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

layout(rgba16ui) uniform highp writeonly uimage2D arg_0;
void textureStore_b70ded() {
  uint arg_1 = 1u;
  uvec4 arg_2 = uvec4(1u);
  imageStore(arg_0, ivec2(uvec2(arg_1, 0u)), arg_2);
}

void compute_main() {
  textureStore_b70ded();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
