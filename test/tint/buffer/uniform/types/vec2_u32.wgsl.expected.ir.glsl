#version 310 es

uniform uvec2 u;
uvec2 s;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uvec2 x = u;
  s = x;
}
