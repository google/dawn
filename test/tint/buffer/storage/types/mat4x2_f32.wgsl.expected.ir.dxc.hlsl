
ByteAddressBuffer tint_symbol : register(t0);
RWByteAddressBuffer tint_symbol_1 : register(u1);
void v(uint offset, float4x2 obj) {
  tint_symbol_1.Store2((offset + 0u), asuint(obj[0u]));
  tint_symbol_1.Store2((offset + 8u), asuint(obj[1u]));
  tint_symbol_1.Store2((offset + 16u), asuint(obj[2u]));
  tint_symbol_1.Store2((offset + 24u), asuint(obj[3u]));
}

float4x2 v_1(uint offset) {
  float2 v_2 = asfloat(tint_symbol.Load2((offset + 0u)));
  float2 v_3 = asfloat(tint_symbol.Load2((offset + 8u)));
  float2 v_4 = asfloat(tint_symbol.Load2((offset + 16u)));
  return float4x2(v_2, v_3, v_4, asfloat(tint_symbol.Load2((offset + 24u))));
}

[numthreads(1, 1, 1)]
void main() {
  v(0u, v_1(0u));
}

