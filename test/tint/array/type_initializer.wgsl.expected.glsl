#version 310 es

layout(binding = 0, std430) buffer s_block_ssbo {
  int inner;
} s;

void tint_symbol() {
  int x = 42;
  int empty[4] = int[4](0, 0, 0, 0);
  int nonempty[4] = int[4](1, 2, 3, 4);
  int nonempty_with_expr[4] = int[4](1, x, (x + 1), nonempty[3]);
  int nested_empty[2][3][4] = int[2][3][4](int[3][4](int[4](0, 0, 0, 0), int[4](0, 0, 0, 0), int[4](0, 0, 0, 0)), int[3][4](int[4](0, 0, 0, 0), int[4](0, 0, 0, 0), int[4](0, 0, 0, 0)));
  int nested_nonempty[2][3][4] = int[2][3][4](int[3][4](int[4](1, 2, 3, 4), int[4](5, 6, 7, 8), int[4](9, 10, 11, 12)), int[3][4](int[4](13, 14, 15, 16), int[4](17, 18, 19, 20), int[4](21, 22, 23, 24)));
  int tint_symbol_1[4] = int[4](1, 2, x, (x + 1));
  int tint_symbol_2[4] = int[4](5, 6, nonempty[2], (nonempty[3] + 1));
  int tint_symbol_3[3][4] = int[3][4](tint_symbol_1, tint_symbol_2, nonempty);
  int nested_nonempty_with_expr[2][3][4] = int[2][3][4](tint_symbol_3, nested_nonempty[1]);
  int subexpr_empty = 0;
  int subexpr_nonempty = 3;
  int tint_symbol_4[4] = int[4](1, x, (x + 1), nonempty[3]);
  int subexpr_nonempty_with_expr = tint_symbol_4[2];
  int subexpr_nested_empty[4] = int[4](0, 0, 0, 0);
  int subexpr_nested_nonempty[4] = int[4](5, 6, 7, 8);
  int tint_symbol_5[4] = int[4](1, x, (x + 1), nonempty[3]);
  int tint_symbol_6[2][4] = int[2][4](tint_symbol_5, nested_nonempty[1][2]);
  int subexpr_nested_nonempty_with_expr[4] = tint_symbol_6[1];
  s.inner = (((((((((((empty[0] + nonempty[0]) + nonempty_with_expr[0]) + nested_empty[0][0][0]) + nested_nonempty[0][0][0]) + nested_nonempty_with_expr[0][0][0]) + subexpr_empty) + subexpr_nonempty) + subexpr_nonempty_with_expr) + subexpr_nested_empty[0]) + subexpr_nested_nonempty[0]) + subexpr_nested_nonempty_with_expr[0]);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
