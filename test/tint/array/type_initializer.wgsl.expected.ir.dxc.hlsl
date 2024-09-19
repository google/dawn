
RWByteAddressBuffer s : register(u0);
[numthreads(1, 1, 1)]
void main() {
  int x = int(42);
  int v[4] = (int[4])0;
  int v_1[4] = {int(1), int(2), int(3), int(4)};
  int nonempty[4] = v_1;
  int v_2[4] = {int(1), x, (x + int(1)), nonempty[int(3)]};
  int v_3[2][3][4] = (int[2][3][4])0;
  int v_4[2][3][4] = {{{int(1), int(2), int(3), int(4)}, {int(5), int(6), int(7), int(8)}, {int(9), int(10), int(11), int(12)}}, {{int(13), int(14), int(15), int(16)}, {int(17), int(18), int(19), int(20)}, {int(21), int(22), int(23), int(24)}}};
  int v_5[4] = {int(1), int(2), x, (x + int(1))};
  int v_6[4] = v_1;
  int v_7[4] = v_1;
  int v_8[4] = v_5;
  int v_9[4] = {int(5), int(6), v_6[int(2)], (v_7[int(3)] + int(1))};
  int v_10[4] = v_1;
  int nested_nonempty[2][3][4] = v_4;
  int v_11[3][4] = {v_8, v_9, v_10};
  int v_12[3][4] = nested_nonempty[int(1)];
  int v_13[2][3][4] = {v_11, v_12};
  int subexpr_empty = int(0);
  int subexpr_nonempty = int(3);
  int v_14[4] = v_1;
  int v_15[4] = {int(1), x, (x + int(1)), v_14[int(3)]};
  int subexpr_nonempty_with_expr = v_15[int(2)];
  int v_16[4] = (int[4])0;
  int v_17[4] = {int(5), int(6), int(7), int(8)};
  int v_18[4] = v_1;
  int v_19[2][3][4] = v_4;
  int v_20[4] = {int(1), x, (x + int(1)), v_18[int(3)]};
  int v_21[4] = v_19[int(1)][int(2)];
  int v_22[2][4] = {v_20, v_21};
  int v_23[4] = v_22[int(1)];
  int empty[4] = v;
  int v_24[4] = v_1;
  int nonempty_with_expr[4] = v_2;
  int nested_empty[2][3][4] = v_3;
  int v_25[2][3][4] = v_4;
  int nested_nonempty_with_expr[2][3][4] = v_13;
  int subexpr_nested_empty[4] = v_16;
  int subexpr_nested_nonempty[4] = v_17;
  int subexpr_nested_nonempty_with_expr[4] = v_23;
  s.Store(0u, asuint((((((((((((empty[int(0)] + v_24[int(0)]) + nonempty_with_expr[int(0)]) + nested_empty[int(0)][int(0)][int(0)]) + v_25[int(0)][int(0)][int(0)]) + nested_nonempty_with_expr[int(0)][int(0)][int(0)]) + subexpr_empty) + subexpr_nonempty) + subexpr_nonempty_with_expr) + subexpr_nested_empty[int(0)]) + subexpr_nested_nonempty[int(0)]) + subexpr_nested_nonempty_with_expr[int(0)])));
}

