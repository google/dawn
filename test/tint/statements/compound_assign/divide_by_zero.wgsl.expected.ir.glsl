#version 310 es

int a = 0;
float b = 0.0f;
int tint_mod_i32(int lhs, int rhs) {
  int v = ((((rhs == 0) | ((lhs == (-2147483647 - 1)) & (rhs == -1)))) ? (1) : (rhs));
  return (lhs - ((lhs / v) * v));
}
int tint_div_i32(int lhs, int rhs) {
  return (lhs / ((((rhs == 0) | ((lhs == (-2147483647 - 1)) & (rhs == -1)))) ? (1) : (rhs)));
}
void foo(int maybe_zero) {
  a = tint_div_i32(a, maybe_zero);
  a = tint_mod_i32(a, maybe_zero);
  b = (b / 0.0f);
  b = (b % 0.0f);
  float v_1 = float(maybe_zero);
  b = (b / v_1);
  float v_2 = float(maybe_zero);
  b = (b % v_2);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
