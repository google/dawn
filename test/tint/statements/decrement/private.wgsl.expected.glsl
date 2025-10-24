#version 310 es

int i = 0;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uint v = uint(i);
  i = int((v - uint(1)));
}
