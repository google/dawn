#version 310 es

int f(int a, int b, int c) {
  uint v = uint(a);
  uint v_1 = uint(int((v * uint(b))));
  return int((v_1 + uint(c)));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  int v_2 = f(1, 2, 3);
  int v_3 = f(4, 5, 6);
  int v_4 = f(7, f(8, 9, 10), 11);
  uint v_5 = uint(v_3);
  int v_6 = int((v_5 * uint(v_4)));
  uint v_7 = uint(v_2);
  int v_8 = int((v_7 + uint(v_6)));
}
