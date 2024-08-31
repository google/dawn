#version 310 es

uniform mat4 u;
mat4 s;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  mat4 x = u;
  s = x;
}
