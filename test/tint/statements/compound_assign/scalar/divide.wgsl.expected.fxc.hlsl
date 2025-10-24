
RWByteAddressBuffer v : register(u0);
int tint_div_i32(int lhs, int rhs) {
  return (lhs / ((((rhs == int(0)) | ((lhs == int(-2147483648)) & (rhs == int(-1))))) ? (int(1)) : (rhs)));
}

[numthreads(1, 1, 1)]
void foo() {
  v.Store(0u, asuint(tint_div_i32(asint(v.Load(0u)), int(2))));
}

