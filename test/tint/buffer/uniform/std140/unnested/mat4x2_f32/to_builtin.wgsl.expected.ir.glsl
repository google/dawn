#version 310 es

uniform mat4x2 u;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  mat2x4 t = transpose(u);
  float l = length(u[1]);
  float a = abs(u[0].yx[0u]);
}
