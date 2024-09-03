#version 310 es

uniform mat4 u;
mat4 p = mat4(vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f));
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  p = u;
  p[1] = u[0];
  p[1] = u[0].ywxz;
  p[0][1] = u[1].x;
}
