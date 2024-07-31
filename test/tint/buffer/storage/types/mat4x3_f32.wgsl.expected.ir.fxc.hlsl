
ByteAddressBuffer tint_symbol : register(t0);
RWByteAddressBuffer tint_symbol_1 : register(u1);
void v(uint offset, float4x3 obj) {
  tint_symbol_1.Store3((offset + 0u), asuint(obj[0u]));
  tint_symbol_1.Store3((offset + 16u), asuint(obj[1u]));
  tint_symbol_1.Store3((offset + 32u), asuint(obj[2u]));
  tint_symbol_1.Store3((offset + 48u), asuint(obj[3u]));
}

float4x3 v_1(uint offset) {
  float3 v_2 = asfloat(tint_symbol.Load3((offset + 0u)));
  float3 v_3 = asfloat(tint_symbol.Load3((offset + 16u)));
  float3 v_4 = asfloat(tint_symbol.Load3((offset + 32u)));
  return float4x3(v_2, v_3, v_4, asfloat(tint_symbol.Load3((offset + 48u))));
}

[numthreads(1, 1, 1)]
void main() {
  v(0u, v_1(0u));
}

