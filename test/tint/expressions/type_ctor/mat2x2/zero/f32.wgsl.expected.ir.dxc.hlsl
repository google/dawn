
static float2x2 m = float2x2((0.0f).xx, (0.0f).xx);
RWByteAddressBuffer tint_symbol : register(u0);
void v(uint offset, float2x2 obj) {
  tint_symbol.Store2((offset + 0u), asuint(obj[0u]));
  tint_symbol.Store2((offset + 8u), asuint(obj[1u]));
}

[numthreads(1, 1, 1)]
void f() {
  v(0u, m);
}

