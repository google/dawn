#version 310 es

uniform mat3x4 u[4];
float s;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  mat4x3 t = transpose(u[2]);
  float l = length(u[0][1].ywxz);
  float a = abs(u[0][1].ywxz[0u]);
  float v = (t[0][0u] + float(l));
  s = (v + float(a));
}
