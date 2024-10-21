#version 310 es

uniform highp isampler2D arg_0;
void d() {
  ivec2 v = ivec2(ivec2(1, 0));
  texelFetch(arg_0, v, int(0));
  float l = 0.14112000167369842529f;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
