#version 310 es

uniform vec2 u;
vec2 s;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  vec2 x = u;
  s = x;
}
