#version 310 es

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

void tint_symbol() {
  int x = 42;
  S1 empty = S1(0, 0, 0, 0);
  S1 nonempty = S1(1, 2, 3, 4);
  S1 nonempty_with_expr = S1(1, x, (x + 1), nonempty.d);
  S3 nested_empty = S3(0, S1(0, 0, 0, 0), S2(0, S1(0, 0, 0, 0)));
  S3 nested_nonempty = S3(1, S1(2, 3, 4, 5), S2(6, S1(7, 8, 9, 10)));
  S1 tint_symbol_1 = S1(2, x, (x + 1), nested_nonempty.i.f.d);
  S2 tint_symbol_2 = S2(6, nonempty);
  S3 nested_nonempty_with_expr = S3(1, tint_symbol_1, tint_symbol_2);
  int subexpr_empty = 0;
  int subexpr_nonempty = 2;
  S1 tint_symbol_3 = S1(1, x, (x + 1), nonempty.d);
  int subexpr_nonempty_with_expr = tint_symbol_3.c;
  S1 subexpr_nested_empty = S1(0, 0, 0, 0);
  S1 subexpr_nested_nonempty = S1(2, 3, 4, 5);
  S1 tint_symbol_4 = S1(2, x, (x + 1), nested_nonempty.i.f.d);
  S2 tint_symbol_5 = S2(1, tint_symbol_4);
  S1 subexpr_nested_nonempty_with_expr = tint_symbol_5.f;
  T aosoa_empty[2] = T[2](T(int[2](0, 0)), T(int[2](0, 0)));
  T aosoa_nonempty[2] = T[2](T(int[2](1, 2)), T(int[2](3, 4)));
  int tint_symbol_6[2] = int[2](1, (aosoa_nonempty[0].a[0] + 1));
  T tint_symbol_7 = T(tint_symbol_6);
  T aosoa_nonempty_with_expr[2] = T[2](tint_symbol_7, aosoa_nonempty[1]);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
