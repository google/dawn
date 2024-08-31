#version 310 es

uniform vec4 u;
vec4 s;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  vec4 x = u;
  s = x;
}
