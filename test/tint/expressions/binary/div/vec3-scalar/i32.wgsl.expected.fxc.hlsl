int3 tint_div(int3 lhs, int rhs) {
  const int3 r = int3((rhs).xxx);
  return (lhs / (((r == (0).xxx) | ((lhs == (-2147483648).xxx) & (r == (-1).xxx))) ? (1).xxx : r));
}

[numthreads(1, 1, 1)]
void f() {
  const int3 a = int3(1, 2, 3);
  const int b = 4;
  const int3 r = tint_div(a, b);
  return;
}
