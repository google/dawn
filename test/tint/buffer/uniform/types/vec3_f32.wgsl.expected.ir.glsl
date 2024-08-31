#version 310 es

uniform vec3 u;
vec3 s;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  vec3 x = u;
  s = x;
}
