#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  int a = 2147483647;
  int b = 1;
  uint v = uint(a);
  int c = int((v + uint(1)));
}
