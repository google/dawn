
static float2x4 m = float2x4((0.0f).xxxx, (0.0f).xxxx);
RWByteAddressBuffer tint_symbol : register(u0);
void v(uint offset, float2x4 obj) {
  tint_symbol.Store4((offset + 0u), asuint(obj[0u]));
  tint_symbol.Store4((offset + 16u), asuint(obj[1u]));
}

[numthreads(1, 1, 1)]
void f() {
  v(0u, m);
}

