
static float4x2 m = float4x2((0.0f).xx, (0.0f).xx, (0.0f).xx, (0.0f).xx);
RWByteAddressBuffer tint_symbol : register(u0);
void v(uint offset, float4x2 obj) {
  tint_symbol.Store2((offset + 0u), asuint(obj[0u]));
  tint_symbol.Store2((offset + 8u), asuint(obj[1u]));
  tint_symbol.Store2((offset + 16u), asuint(obj[2u]));
  tint_symbol.Store2((offset + 24u), asuint(obj[3u]));
}

[numthreads(1, 1, 1)]
void f() {
  v(0u, m);
}

