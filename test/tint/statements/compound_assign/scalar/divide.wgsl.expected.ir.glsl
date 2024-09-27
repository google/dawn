#version 310 es


struct S {
  int a;
};

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  S tint_symbol;
} v_1;
int tint_div_i32(int lhs, int rhs) {
  uint v_2 = uint((lhs == (-2147483647 - 1)));
  bool v_3 = bool((v_2 & uint((rhs == -1))));
  uint v_4 = uint((rhs == 0));
  return (lhs / mix(rhs, 1, bool((v_4 | uint(v_3)))));
}
void foo() {
  v_1.tint_symbol.a = tint_div_i32(v_1.tint_symbol.a, 2);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
