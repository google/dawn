#version 310 es

void f_1() {
  int v = 0;
  int n = 0;
  uint offset_1 = 0u;
  uint count = 0u;
  int v_1 = v;
  int v_2 = n;
  uint v_3 = count;
  uint v_4 = min(offset_1, 32u);
  uint v_5 = min(v_3, (32u - v_4));
  int v_6 = int(v_4);
  int x_15 = bitfieldInsert(v_1, v_2, v_6, int(v_5));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f_1();
}
