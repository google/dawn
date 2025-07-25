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

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  int x = 42;
  S1 empty = S1(0, 0, 0, 0);
  S1 nonempty = S1(1, 2, 3, 4);
  uint v = uint(x);
  S1 nonempty_with_expr = S1(1, x, int((v + uint(1))), nonempty.d);
  S3 nested_empty = S3(0, S1(0, 0, 0, 0), S2(0, S1(0, 0, 0, 0)));
  S3 nested_nonempty = S3(1, S1(2, 3, 4, 5), S2(6, S1(7, 8, 9, 10)));
  uint v_1 = uint(x);
  S1 v_2 = S1(2, x, int((v_1 + uint(1))), nested_nonempty.i.f.d);
  S3 nested_nonempty_with_expr = S3(1, v_2, S2(6, nonempty));
  int subexpr_empty = 0;
  int subexpr_nonempty = 2;
  uint v_3 = uint(x);
  int subexpr_nonempty_with_expr = S1(1, x, int((v_3 + uint(1))), nonempty.d).c;
  S1 subexpr_nested_empty = S1(0, 0, 0, 0);
  S1 subexpr_nested_nonempty = S1(2, 3, 4, 5);
  uint v_4 = uint(x);
  S1 subexpr_nested_nonempty_with_expr = S2(1, S1(2, x, int((v_4 + uint(1))), nested_nonempty.i.f.d)).f;
  T aosoa_empty[2] = T[2](T(int[2](0, 0)), T(int[2](0, 0)));
  T aosoa_nonempty[2] = T[2](T(int[2](1, 2)), T(int[2](3, 4)));
  uint v_5 = uint(aosoa_nonempty[0u].a[0u]);
  T aosoa_nonempty_with_expr[2] = T[2](T(int[2](1, int((v_5 + uint(1))))), aosoa_nonempty[1u]);
}
