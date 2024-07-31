
RWByteAddressBuffer tint_symbol : register(u0);
void v(uint offset, float3x4 obj) {
  tint_symbol.Store4((offset + 0u), asuint(obj[0u]));
  tint_symbol.Store4((offset + 16u), asuint(obj[1u]));
  tint_symbol.Store4((offset + 32u), asuint(obj[2u]));
}

[numthreads(1, 1, 1)]
void f() {
  float3x4 m = float3x4((0.0f).xxxx, (0.0f).xxxx, (0.0f).xxxx);
  v(0u, float3x4(m));
}

