#version 310 es

int c(int x, int y, int z) {
  uint v = uint(int((1u + uint(x))));
  uint v_1 = uint(int((v + uint(y))));
  int a = int((v_1 + uint(z)));
  a = int((uint(a) + 2u));
  return a;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  int b_1 = c(2, 3, 4);
  int v_2 = b_1;
  int v_3 = c(3, 4, 5);
  uint v_4 = uint(v_2);
  b_1 = int((v_4 + uint(v_3)));
}
