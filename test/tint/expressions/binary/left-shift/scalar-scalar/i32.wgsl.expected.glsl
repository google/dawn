#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  int a = 1;
  uint b = 2u;
  int r = int((uint(a) << (b & 31u)));
}
