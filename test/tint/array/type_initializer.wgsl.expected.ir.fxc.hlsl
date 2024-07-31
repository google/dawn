
RWByteAddressBuffer s : register(u0);
[numthreads(1, 1, 1)]
void main() {
  int x = 42;
  int v[4] = (int[4])0;
  int v_1[4] = {1, 2, 3, 4};
  int nonempty[4] = v_1;
  int v_2[4] = {1, x, (x + 1), nonempty[3]};
  int v_3[2][3][4] = (int[2][3][4])0;
  int v_4[2][3][4] = {{{1, 2, 3, 4}, {5, 6, 7, 8}, {9, 10, 11, 12}}, {{13, 14, 15, 16}, {17, 18, 19, 20}, {21, 22, 23, 24}}};
  int v_5[4] = {1, 2, x, (x + 1)};
  int v_6[4] = v_1;
  int v_7[4] = v_1;
  int v_8[4] = v_5;
  int v_9[4] = {5, 6, v_6[2], (v_7[3] + 1)};
  int v_10[4] = v_1;
  int nested_nonempty[2][3][4] = v_4;
  int v_11[3][4] = {v_8, v_9, v_10};
  int v_12[3][4] = nested_nonempty[1];
  int v_13[2][3][4] = {v_11, v_12};
  int subexpr_empty = 0;
  int subexpr_nonempty = 3;
  int v_14[4] = v_1;
  int v_15[4] = {1, x, (x + 1), v_14[3]};
  int subexpr_nonempty_with_expr = v_15[2];
  int v_16[4] = (int[4])0;
  int v_17[4] = {5, 6, 7, 8};
  int v_18[4] = v_1;
  int v_19[2][3][4] = v_4;
  int v_20[4] = {1, x, (x + 1), v_18[3]};
  int v_21[4] = v_19[1][2];
  int v_22[2][4] = {v_20, v_21};
  int v_23[4] = v_22[1];
  int empty[4] = v;
  int v_24[4] = v_1;
  int nonempty_with_expr[4] = v_2;
  int nested_empty[2][3][4] = v_3;
  int v_25[2][3][4] = v_4;
  int nested_nonempty_with_expr[2][3][4] = v_13;
  int subexpr_nested_empty[4] = v_16;
  int subexpr_nested_nonempty[4] = v_17;
  int subexpr_nested_nonempty_with_expr[4] = v_23;
  s.Store(0u, asuint((((((((((((empty[0] + v_24[0]) + nonempty_with_expr[0]) + nested_empty[0][0][0]) + v_25[0][0][0]) + nested_nonempty_with_expr[0][0][0]) + subexpr_empty) + subexpr_nonempty) + subexpr_nonempty_with_expr) + subexpr_nested_empty[0]) + subexpr_nested_nonempty[0]) + subexpr_nested_nonempty_with_expr[0])));
}

