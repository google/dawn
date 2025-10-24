#version 310 es

int c(int z) {
  uint v = uint(1);
  int a = int((v + uint(z)));
  uint v_1 = uint(a);
  a = int((v_1 + uint(2)));
  return a;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  int b_1 = c(2);
  int v_2 = c(3);
  uint v_3 = uint(b_1);
  b_1 = int((v_3 + uint(v_2)));
}
