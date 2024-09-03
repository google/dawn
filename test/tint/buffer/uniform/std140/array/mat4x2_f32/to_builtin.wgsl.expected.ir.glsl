#version 310 es

uniform mat4x2 u[4];
float s;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  mat2x4 t = transpose(u[2]);
  float l = length(u[0][1].yx);
  float a = abs(u[0][1].yx[0u]);
  float v = (t[0][0u] + float(l));
  s = (v + float(a));
}
