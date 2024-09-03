#version 310 es

uniform mat4 u;
mat4 s;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  s = u;
  s[1] = u[0];
  s[1] = u[0].ywxz;
  s[0][1] = u[1].x;
}
