#version 310 es

uniform ivec3 u;
ivec3 s;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  ivec3 x = u;
  s = x;
}
