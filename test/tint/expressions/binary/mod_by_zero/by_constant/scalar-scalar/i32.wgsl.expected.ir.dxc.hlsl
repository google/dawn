
int tint_mod_i32(int lhs, int rhs) {
  int v = ((((rhs == 0) | ((lhs == -2147483648) & (rhs == -1)))) ? (1) : (rhs));
  return (lhs - ((lhs / v) * v));
}

[numthreads(1, 1, 1)]
void f() {
  int a = 1;
  int b = 0;
  int r = tint_mod_i32(a, b);
}

