
static float4x3 m = float4x3((0.0f).xxx, (0.0f).xxx, (0.0f).xxx, (0.0f).xxx);
RWByteAddressBuffer tint_symbol : register(u0);
void v(uint offset, float4x3 obj) {
  tint_symbol.Store3((offset + 0u), asuint(obj[0u]));
  tint_symbol.Store3((offset + 16u), asuint(obj[1u]));
  tint_symbol.Store3((offset + 32u), asuint(obj[2u]));
  tint_symbol.Store3((offset + 48u), asuint(obj[3u]));
}

[numthreads(1, 1, 1)]
void f() {
  v(0u, m);
}

