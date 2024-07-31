
RWByteAddressBuffer tint_symbol : register(u0);
void v(uint offset, float2x2 obj) {
  tint_symbol.Store2((offset + 0u), asuint(obj[0u]));
  tint_symbol.Store2((offset + 8u), asuint(obj[1u]));
}

[numthreads(1, 1, 1)]
void f() {
  v(0u, float2x2(float2(0.0f, 1.0f), float2(2.0f, 3.0f)));
}

