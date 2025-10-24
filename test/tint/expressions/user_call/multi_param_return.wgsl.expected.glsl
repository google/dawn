#version 310 es

int c(int x, int y, int z) {
  uint v = uint(1);
  uint v_1 = uint(int((v + uint(x))));
  uint v_2 = uint(int((v_1 + uint(y))));
  int a = int((v_2 + uint(z)));
  uint v_3 = uint(a);
  a = int((v_3 + uint(2)));
  return a;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  int b_1 = c(2, 3, 4);
  int v_4 = c(3, 4, 5);
  uint v_5 = uint(b_1);
  b_1 = int((v_5 + uint(v_4)));
}
