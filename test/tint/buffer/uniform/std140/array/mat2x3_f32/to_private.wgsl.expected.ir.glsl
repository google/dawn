#version 310 es

uniform mat2x3 u[4];
float s;
mat2x3 p[4] = mat2x3[4](mat2x3(vec3(0.0f), vec3(0.0f)), mat2x3(vec3(0.0f), vec3(0.0f)), mat2x3(vec3(0.0f), vec3(0.0f)), mat2x3(vec3(0.0f), vec3(0.0f)));
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  p = u;
  p[1] = u[2];
  p[1][0] = u[0][1].zxy;
  p[1][0][0u] = u[0][1].x;
  s = p[1][0].x;
}
