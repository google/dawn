
RWByteAddressBuffer v : register(u0);
int tint_mod_i32(int lhs, int rhs) {
  int v_1 = ((((rhs == 0) | ((lhs == -2147483648) & (rhs == -1)))) ? (1) : (rhs));
  return (lhs - ((lhs / v_1) * v_1));
}

void foo() {
  v.Store(0u, asuint(tint_mod_i32(asint(v.Load(0u)), 2)));
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

