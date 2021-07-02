struct tint_array_wrapper {
  int arr[4];
};
struct tint_array_wrapper_2 {
  tint_array_wrapper arr[3];
};
struct tint_array_wrapper_1 {
  tint_array_wrapper_2 arr[2];
};
struct tint_array_wrapper_3 {
  tint_array_wrapper arr[2];
};

[numthreads(1, 1, 1)]
void main() {
  const int x = 42;
  const tint_array_wrapper empty = {(int[4])0};
  const tint_array_wrapper nonempty = {{1, 2, 3, 4}};
  const tint_array_wrapper nonempty_with_expr = {{1, x, (x + 1), nonempty.arr[3]}};
  const tint_array_wrapper_1 nested_empty = {(tint_array_wrapper_2[2])0};
  const tint_array_wrapper tint_symbol = {{1, 2, 3, 4}};
  const tint_array_wrapper tint_symbol_1 = {{5, 6, 7, 8}};
  const tint_array_wrapper tint_symbol_2 = {{9, 10, 11, 12}};
  const tint_array_wrapper_2 tint_symbol_3 = {{tint_symbol, tint_symbol_1, tint_symbol_2}};
  const tint_array_wrapper tint_symbol_4 = {{13, 14, 15, 16}};
  const tint_array_wrapper tint_symbol_5 = {{17, 18, 19, 20}};
  const tint_array_wrapper tint_symbol_6 = {{21, 22, 23, 24}};
  const tint_array_wrapper_2 tint_symbol_7 = {{tint_symbol_4, tint_symbol_5, tint_symbol_6}};
  const tint_array_wrapper_1 nested_nonempty = {{tint_symbol_3, tint_symbol_7}};
  const tint_array_wrapper tint_symbol_8 = {{1, 2, x, (x + 1)}};
  const tint_array_wrapper tint_symbol_9 = {{5, 6, nonempty.arr[2], (nonempty.arr[3] + 1)}};
  const tint_array_wrapper_2 tint_symbol_10 = {{tint_symbol_8, tint_symbol_9, nonempty}};
  const tint_array_wrapper_1 nested_nonempty_with_expr = {{tint_symbol_10, nested_nonempty.arr[1]}};
  const tint_array_wrapper tint_symbol_11 = {(int[4])0};
  const int subexpr_empty = tint_symbol_11.arr[1];
  const tint_array_wrapper tint_symbol_12 = {{1, 2, 3, 4}};
  const int subexpr_nonempty = tint_symbol_12.arr[2];
  const tint_array_wrapper tint_symbol_13 = {{1, x, (x + 1), nonempty.arr[3]}};
  const int subexpr_nonempty_with_expr = tint_symbol_13.arr[2];
  const tint_array_wrapper_3 tint_symbol_14 = {(tint_array_wrapper[2])0};
  const tint_array_wrapper subexpr_nested_empty = tint_symbol_14.arr[1];
  const tint_array_wrapper tint_symbol_15 = {{1, 2, 3, 4}};
  const tint_array_wrapper tint_symbol_16 = {{5, 6, 7, 8}};
  const tint_array_wrapper_3 tint_symbol_17 = {{tint_symbol_15, tint_symbol_16}};
  const tint_array_wrapper subexpr_nested_nonempty = tint_symbol_17.arr[1];
  const tint_array_wrapper tint_symbol_18 = {{1, x, (x + 1), nonempty.arr[3]}};
  const tint_array_wrapper_3 tint_symbol_19 = {{tint_symbol_18, nested_nonempty.arr[1].arr[2]}};
  const tint_array_wrapper subexpr_nested_nonempty_with_expr = tint_symbol_19.arr[1];
  return;
}
