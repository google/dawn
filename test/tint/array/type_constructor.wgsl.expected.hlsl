[numthreads(1, 1, 1)]
void main() {
  const int x = 42;
  const int empty[4] = (int[4])0;
  const int nonempty[4] = {1, 2, 3, 4};
  const int nonempty_with_expr[4] = {1, x, (x + 1), 4};
  const int nested_empty[2][3][4] = (int[2][3][4])0;
  const int tint_symbol[4] = {1, 2, 3, 4};
  const int tint_symbol_1[4] = {5, 6, 7, 8};
  const int tint_symbol_2[4] = {9, 10, 11, 12};
  const int tint_symbol_3[3][4] = {{1, 2, 3, 4}, {5, 6, 7, 8}, {9, 10, 11, 12}};
  const int tint_symbol_4[4] = {13, 14, 15, 16};
  const int tint_symbol_5[4] = {17, 18, 19, 20};
  const int tint_symbol_6[4] = {21, 22, 23, 24};
  const int tint_symbol_7[3][4] = {{13, 14, 15, 16}, {17, 18, 19, 20}, {21, 22, 23, 24}};
  const int nested_nonempty[2][3][4] = {{{1, 2, 3, 4}, {5, 6, 7, 8}, {9, 10, 11, 12}}, {{13, 14, 15, 16}, {17, 18, 19, 20}, {21, 22, 23, 24}}};
  const int tint_symbol_8[4] = {1, 2, x, (x + 1)};
  const int tint_symbol_9[4] = {5, 6, 3, (4 + 1)};
  const int tint_symbol_10[3][4] = {tint_symbol_8, tint_symbol_9, nonempty};
  const int nested_nonempty_with_expr[2][3][4] = {tint_symbol_10, {{13, 14, 15, 16}, {17, 18, 19, 20}, {21, 22, 23, 24}}};
  const int tint_symbol_11[4] = (int[4])0;
  const int subexpr_empty = 0;
  const int tint_symbol_12[4] = {1, 2, 3, 4};
  const int subexpr_nonempty = 3;
  const int tint_symbol_13[4] = {1, x, (x + 1), 4};
  const int subexpr_nonempty_with_expr = tint_symbol_13[2];
  const int tint_symbol_14[2][4] = (int[2][4])0;
  const int subexpr_nested_empty[4] = (int[4])0;
  const int tint_symbol_15[4] = {1, 2, 3, 4};
  const int tint_symbol_16[4] = {5, 6, 7, 8};
  const int tint_symbol_17[2][4] = {{1, 2, 3, 4}, {5, 6, 7, 8}};
  const int subexpr_nested_nonempty[4] = {5, 6, 7, 8};
  const int tint_symbol_18[4] = {1, x, (x + 1), 4};
  const int tint_symbol_19[2][4] = {tint_symbol_18, {21, 22, 23, 24}};
  const int subexpr_nested_nonempty_with_expr[4] = tint_symbol_19[1];
  return;
}
