
ByteAddressBuffer tint_symbol : register(t0);
RWByteAddressBuffer tint_symbol_1 : register(u1);
void v(uint offset, float2x3 obj) {
  tint_symbol_1.Store3((offset + 0u), asuint(obj[0u]));
  tint_symbol_1.Store3((offset + 16u), asuint(obj[1u]));
}

float2x3 v_1(uint offset) {
  float3 v_2 = asfloat(tint_symbol.Load3((offset + 0u)));
  return float2x3(v_2, asfloat(tint_symbol.Load3((offset + 16u))));
}

[numthreads(1, 1, 1)]
void main() {
  v(0u, v_1(0u));
}

