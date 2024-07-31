
static float3x3 m = float3x3(float3(0.0f, 1.0f, 2.0f), float3(3.0f, 4.0f, 5.0f), float3(6.0f, 7.0f, 8.0f));
RWByteAddressBuffer tint_symbol : register(u0);
void v(uint offset, float3x3 obj) {
  tint_symbol.Store3((offset + 0u), asuint(obj[0u]));
  tint_symbol.Store3((offset + 16u), asuint(obj[1u]));
  tint_symbol.Store3((offset + 32u), asuint(obj[2u]));
}

[numthreads(1, 1, 1)]
void f() {
  v(0u, float3x3(m));
}

