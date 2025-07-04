#version 310 es

void v() {
  int b = 2;
  int v_1 = b;
  int v_2 = int((~(uint(b)) + 1u));
  uint v_3 = uint(v_1);
  int c = int((v_3 - uint(v_2)));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
