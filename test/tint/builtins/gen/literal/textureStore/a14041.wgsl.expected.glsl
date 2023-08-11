#version 310 es

layout(rgba32i) uniform highp writeonly iimage2D arg_0;
void textureStore_a14041() {
  imageStore(arg_0, ivec2(1, 0), ivec4(1));
}

vec4 vertex_main() {
  textureStore_a14041();
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

layout(rgba32i) uniform highp writeonly iimage2D arg_0;
void textureStore_a14041() {
  imageStore(arg_0, ivec2(1, 0), ivec4(1));
}

void fragment_main() {
  textureStore_a14041();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

layout(rgba32i) uniform highp writeonly iimage2D arg_0;
void textureStore_a14041() {
  imageStore(arg_0, ivec2(1, 0), ivec4(1));
}

void compute_main() {
  textureStore_a14041();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
