
RWByteAddressBuffer tint_symbol : register(u0);
void v(uint offset, float3x2 obj) {
  tint_symbol.Store2((offset + 0u), asuint(obj[0u]));
  tint_symbol.Store2((offset + 8u), asuint(obj[1u]));
  tint_symbol.Store2((offset + 16u), asuint(obj[2u]));
}

[numthreads(1, 1, 1)]
void f() {
  float3x2 m = float3x2((0.0f).xx, (0.0f).xx, (0.0f).xx);
  v(0u, float3x2(m));
}

