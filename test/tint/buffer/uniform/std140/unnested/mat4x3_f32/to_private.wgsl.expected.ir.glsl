#version 310 es

uniform mat4x3 u;
mat4x3 p = mat4x3(vec3(0.0f), vec3(0.0f), vec3(0.0f), vec3(0.0f));
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  p = u;
  p[1] = u[0];
  p[1] = u[0].zxy;
  p[0][1] = u[1].x;
}
