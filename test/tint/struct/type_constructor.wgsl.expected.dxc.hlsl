struct S1 {
  int a;
  int b;
  int c;
  int d;
};
struct S2 {
  int e;
  S1 f;
};
struct S3 {
  int g;
  S1 h;
  S2 i;
};
struct T {
  int a[2];
};

[numthreads(1, 1, 1)]
void main() {
  const int x = 42;
  const S1 empty = (S1)0;
  const S1 nonempty = {1, 2, 3, 4};
  const S1 nonempty_with_expr = {1, x, (x + 1), nonempty.d};
  const S3 nested_empty = (S3)0;
  const S1 tint_symbol = {2, 3, 4, 5};
  const S1 tint_symbol_1 = {7, 8, 9, 10};
  const S2 tint_symbol_2 = {6, tint_symbol_1};
  const S3 nested_nonempty = {1, tint_symbol, tint_symbol_2};
  const S1 tint_symbol_3 = {2, x, (x + 1), nested_nonempty.i.f.d};
  const S2 tint_symbol_4 = {6, nonempty};
  const S3 nested_nonempty_with_expr = {1, tint_symbol_3, tint_symbol_4};
  const S1 tint_symbol_5 = (S1)0;
  const int subexpr_empty = tint_symbol_5.a;
  const S1 tint_symbol_6 = {1, 2, 3, 4};
  const int subexpr_nonempty = tint_symbol_6.b;
  const S1 tint_symbol_7 = {1, x, (x + 1), nonempty.d};
  const int subexpr_nonempty_with_expr = tint_symbol_7.c;
  const S2 tint_symbol_8 = (S2)0;
  const S1 subexpr_nested_empty = tint_symbol_8.f;
  const S1 tint_symbol_9 = {2, 3, 4, 5};
  const S2 tint_symbol_10 = {1, tint_symbol_9};
  const S1 subexpr_nested_nonempty = tint_symbol_10.f;
  const S1 tint_symbol_11 = {2, x, (x + 1), nested_nonempty.i.f.d};
  const S2 tint_symbol_12 = {1, tint_symbol_11};
  const S1 subexpr_nested_nonempty_with_expr = tint_symbol_12.f;
  const T aosoa_empty[2] = (T[2])0;
  const int tint_symbol_13[2] = {1, 2};
  const T tint_symbol_14 = {tint_symbol_13};
  const int tint_symbol_15[2] = {3, 4};
  const T tint_symbol_16 = {tint_symbol_15};
  const T aosoa_nonempty[2] = {tint_symbol_14, tint_symbol_16};
  const int tint_symbol_17[2] = {1, (aosoa_nonempty[0].a[0] + 1)};
  const T tint_symbol_18 = {tint_symbol_17};
  const T aosoa_nonempty_with_expr[2] = {tint_symbol_18, aosoa_nonempty[1]};
  return;
}
