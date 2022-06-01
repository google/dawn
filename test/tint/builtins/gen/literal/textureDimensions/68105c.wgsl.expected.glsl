#version 310 es

layout(rgba32ui) uniform highp writeonly uimage2D arg_0;
void textureDimensions_68105c() {
  ivec2 res = imageSize(arg_0);
}

vec4 vertex_main() {
  textureDimensions_68105c();
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
precision mediump float;

layout(rgba32ui) uniform highp writeonly uimage2D arg_0;
void textureDimensions_68105c() {
  ivec2 res = imageSize(arg_0);
}

void fragment_main() {
  textureDimensions_68105c();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

layout(rgba32ui) uniform highp writeonly uimage2D arg_0;
void textureDimensions_68105c() {
  ivec2 res = imageSize(arg_0);
}

void compute_main() {
  textureDimensions_68105c();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
