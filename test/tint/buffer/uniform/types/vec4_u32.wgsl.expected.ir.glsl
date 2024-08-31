#version 310 es

uniform uvec4 u;
uvec4 s;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uvec4 x = u;
  s = x;
}
