#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  int i = 123;
  uint v = uint(i);
  int u = int((v + uint(1)));
}
