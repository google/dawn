
RWByteAddressBuffer tint_symbol : register(u0);
void v(uint offset, float4x2 obj) {
  tint_symbol.Store2((offset + 0u), asuint(obj[0u]));
  tint_symbol.Store2((offset + 8u), asuint(obj[1u]));
  tint_symbol.Store2((offset + 16u), asuint(obj[2u]));
  tint_symbol.Store2((offset + 24u), asuint(obj[3u]));
}

[numthreads(1, 1, 1)]
void f() {
  v(0u, float4x2(float2(0.0f, 1.0f), float2(2.0f, 3.0f), float2(4.0f, 5.0f), float2(6.0f, 7.0f)));
}

