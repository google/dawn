#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
uniform highp isampler2D arg_0_1;
void d() {
  texelFetch(arg_0_1, ivec2(1, 0), 0);
  float l = 0.141120002f;
}

