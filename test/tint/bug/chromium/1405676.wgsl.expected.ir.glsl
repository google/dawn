#version 310 es

uniform highp isampler1D arg_0;
void d() {
  int v = int(1);
  texelFetch(arg_0, v, int(0));
  float l = 0.14112000167369842529f;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
