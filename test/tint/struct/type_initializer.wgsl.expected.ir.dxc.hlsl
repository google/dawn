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
  int x = 42;
  S1 empty = (S1)0;
  S1 v = {1, 2, 3, 4};
  S1 nonempty = v;
  S1 nonempty_with_expr = {1, x, (x + 1), nonempty.d};
  S3 nested_empty = (S3)0;
  S3 v_1 = {1, {2, 3, 4, 5}, {6, {7, 8, 9, 10}}};
  S3 nested_nonempty = v_1;
  S1 v_2 = {2, x, (x + 1), nested_nonempty.i.f.d};
  S1 v_3 = v;
  S1 v_4 = v_2;
  S2 v_5 = {6, v_3};
  S3 nested_nonempty_with_expr = {1, v_4, v_5};
  int subexpr_empty = 0;
  int subexpr_nonempty = 2;
  S1 v_6 = v;
  S1 v_7 = {1, x, (x + 1), v_6.d};
  int subexpr_nonempty_with_expr = v_7.c;
  S1 subexpr_nested_empty = (S1)0;
  S1 subexpr_nested_nonempty = {2, 3, 4, 5};
  S3 v_8 = v_1;
  S1 v_9 = {2, x, (x + 1), v_8.i.f.d};
  S2 v_10 = {1, v_9};
  S1 subexpr_nested_nonempty_with_expr = v_10.f;
  T aosoa_empty[2] = (T[2])0;
  T v_11[2] = {{{1, 2}}, {{3, 4}}};
  T aosoa_nonempty[2] = v_11;
  int v_12[2] = {1, (aosoa_nonempty[0].a[0] + 1)};
  T v_13[2] = v_11;
  T v_14 = {v_12};
  T v_15 = v_13[1];
  T aosoa_nonempty_with_expr[2] = {v_14, v_15};
}

