#version 310 es

uniform int u;
int s;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  int x = u;
  s = x;
}
