#version 310 es

void c() {
  int a = 1;
  uint v = uint(a);
  a = int((v + uint(2)));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  c();
  c();
}
