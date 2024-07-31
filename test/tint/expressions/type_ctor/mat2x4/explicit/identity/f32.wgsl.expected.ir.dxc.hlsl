
static float2x4 m = float2x4(float4(0.0f, 1.0f, 2.0f, 3.0f), float4(4.0f, 5.0f, 6.0f, 7.0f));
RWByteAddressBuffer tint_symbol : register(u0);
void v(uint offset, float2x4 obj) {
  tint_symbol.Store4((offset + 0u), asuint(obj[0u]));
  tint_symbol.Store4((offset + 16u), asuint(obj[1u]));
}

[numthreads(1, 1, 1)]
void f() {
  v(0u, float2x4(m));
}

