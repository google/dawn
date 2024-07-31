
int3 tint_mod_v3i32(int3 lhs, int3 rhs) {
  int3 v = ((((rhs == (0).xxx) | ((lhs == (-2147483648).xxx) & (rhs == (-1).xxx)))) ? ((1).xxx) : (rhs));
  return (lhs - ((lhs / v) * v));
}

[numthreads(1, 1, 1)]
void f() {
  int3 a = int3(1, 2, 3);
  int3 b = int3(4, 5, 6);
  int3 r = tint_mod_v3i32(a, b);
}

