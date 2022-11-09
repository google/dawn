int tint_div(int lhs, int rhs) {
  return (lhs / (((rhs == 0) | ((lhs == -2147483648) & (rhs == -1))) ? 1 : rhs));
}

[numthreads(1, 1, 1)]
void f() {
  const int a = 1;
  const int b = 0;
  const int r = tint_div(a, b);
  return;
}
