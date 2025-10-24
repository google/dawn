#version 310 es


struct S {
  int a;
};

layout(binding = 0, std430)
buffer v_block_1_ssbo {
  S inner;
} v_1;
int tint_mod_i32(int lhs, int rhs) {
  uint v_2 = uint((lhs == (-2147483647 - 1)));
  bool v_3 = bool((v_2 & uint((rhs == -1))));
  uint v_4 = uint((rhs == 0));
  int v_5 = mix(rhs, 1, bool((v_4 | uint(v_3))));
  uint v_6 = uint((lhs / v_5));
  int v_7 = int((v_6 * uint(v_5)));
  uint v_8 = uint(lhs);
  return int((v_8 - uint(v_7)));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v_1.inner.a = tint_mod_i32(v_1.inner.a, 2);
}
