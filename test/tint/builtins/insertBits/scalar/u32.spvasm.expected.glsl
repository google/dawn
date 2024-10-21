#version 310 es

void f_1() {
  uint v = 0u;
  uint n = 0u;
  uint offset_1 = 0u;
  uint count = 0u;
  uint v_1 = v;
  uint v_2 = n;
  uint v_3 = count;
  uint v_4 = min(offset_1, 32u);
  uint v_5 = min(v_3, (32u - v_4));
  int v_6 = int(v_4);
  uint x_12 = bitfieldInsert(v_1, v_2, v_6, int(v_5));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f_1();
}
