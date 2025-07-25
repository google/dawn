#version 310 es

void c(int x, int y, int z) {
  uint v = uint(1);
  uint v_1 = uint(int((v + uint(x))));
  uint v_2 = uint(int((v_1 + uint(y))));
  int a = int((v_2 + uint(z)));
  uint v_3 = uint(a);
  a = int((v_3 + uint(2)));
}
void b() {
  c(1, 2, 3);
  c(4, 5, 6);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
