#version 310 es

int s;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  int c = 0;
  int d = 0;
  s = (c + d);
}
