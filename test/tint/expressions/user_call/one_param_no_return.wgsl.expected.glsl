#version 310 es

void c(int z) {
  uint v = uint(1);
  int a = int((v + uint(z)));
  uint v_1 = uint(a);
  a = int((v_1 + uint(2)));
}
void b() {
  c(2);
  c(3);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
