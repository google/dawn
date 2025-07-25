#version 310 es

int tint_mod_i32(int lhs, int rhs) {
  uint v = uint((lhs == (-2147483647 - 1)));
  bool v_1 = bool((v & uint((rhs == -1))));
  uint v_2 = uint((rhs == 0));
  int v_3 = mix(rhs, 1, bool((v_2 | uint(v_1))));
  uint v_4 = uint((lhs / v_3));
  int v_5 = int((v_4 * uint(v_3)));
  uint v_6 = uint(lhs);
  return int((v_6 - uint(v_5)));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  int a = 1;
  int b = 0;
  int v_7 = a;
  int v_8 = b;
  uint v_9 = uint(b);
  int r = tint_mod_i32(v_7, int((v_9 + uint(v_8))));
}
