#version 310 es

uniform mat3x2 u;
mat3x2 s;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  mat3x2 x = u;
  s = x;
}
