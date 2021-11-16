#version 310 es
precision mediump float;

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void tint_symbol() {
  int x = 42;
  int empty[4] = int[4](0, 0, 0, 0);
  int nonempty[4] = int[4](1, 2, 3, 4);
  int nonempty_with_expr[4] = int[4](1, x, (x + 1), nonempty[3]);
  int nested_empty[2][3][4] = int[2][3][4](int[3][4](int[4](0, 0, 0, 0), int[4](0, 0, 0, 0), int[4](0, 0, 0, 0)), int[3][4](int[4](0, 0, 0, 0), int[4](0, 0, 0, 0), int[4](0, 0, 0, 0)));
  int tint_symbol_1[4] = int[4](1, 2, 3, 4);
  int tint_symbol_2[4] = int[4](5, 6, 7, 8);
  int tint_symbol_3[4] = int[4](9, 10, 11, 12);
  int tint_symbol_4[3][4] = int[3][4](tint_symbol_1, tint_symbol_2, tint_symbol_3);
  int tint_symbol_5[4] = int[4](13, 14, 15, 16);
  int tint_symbol_6[4] = int[4](17, 18, 19, 20);
  int tint_symbol_7[4] = int[4](21, 22, 23, 24);
  int tint_symbol_8[3][4] = int[3][4](tint_symbol_5, tint_symbol_6, tint_symbol_7);
  int nested_nonempty[2][3][4] = int[2][3][4](tint_symbol_4, tint_symbol_8);
  int tint_symbol_9[4] = int[4](1, 2, x, (x + 1));
  int tint_symbol_10[4] = int[4](5, 6, nonempty[2], (nonempty[3] + 1));
  int tint_symbol_11[3][4] = int[3][4](tint_symbol_9, tint_symbol_10, nonempty);
  int nested_nonempty_with_expr[2][3][4] = int[2][3][4](tint_symbol_11, nested_nonempty[1]);
  int tint_symbol_12[4] = int[4](0, 0, 0, 0);
  int subexpr_empty = tint_symbol_12[1];
  int tint_symbol_13[4] = int[4](1, 2, 3, 4);
  int subexpr_nonempty = tint_symbol_13[2];
  int tint_symbol_14[4] = int[4](1, x, (x + 1), nonempty[3]);
  int subexpr_nonempty_with_expr = tint_symbol_14[2];
  int tint_symbol_15[2][4] = int[2][4](int[4](0, 0, 0, 0), int[4](0, 0, 0, 0));
  int subexpr_nested_empty[4] = tint_symbol_15[1];
  int tint_symbol_16[4] = int[4](1, 2, 3, 4);
  int tint_symbol_17[4] = int[4](5, 6, 7, 8);
  int tint_symbol_18[2][4] = int[2][4](tint_symbol_16, tint_symbol_17);
  int subexpr_nested_nonempty[4] = tint_symbol_18[1];
  int tint_symbol_19[4] = int[4](1, x, (x + 1), nonempty[3]);
  int tint_symbol_20[2][4] = int[2][4](tint_symbol_19, nested_nonempty[1][2]);
  int subexpr_nested_nonempty_with_expr[4] = tint_symbol_20[1];
  return;
}
void main() {
  tint_symbol();
}


