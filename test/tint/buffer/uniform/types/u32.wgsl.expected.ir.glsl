#version 310 es

uniform uint u;
uint s;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uint x = u;
  s = x;
}
