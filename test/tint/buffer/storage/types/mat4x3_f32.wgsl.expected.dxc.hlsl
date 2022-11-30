ByteAddressBuffer tint_symbol : register(t0, space0);
RWByteAddressBuffer tint_symbol_1 : register(u1, space0);

void tint_symbol_2(RWByteAddressBuffer buffer, uint offset, float4x3 value) {
  buffer.Store3((offset + 0u), asuint(value[0u]));
  buffer.Store3((offset + 16u), asuint(value[1u]));
  buffer.Store3((offset + 32u), asuint(value[2u]));
  buffer.Store3((offset + 48u), asuint(value[3u]));
}

float4x3 tint_symbol_4(ByteAddressBuffer buffer, uint offset) {
  return float4x3(asfloat(buffer.Load3((offset + 0u))), asfloat(buffer.Load3((offset + 16u))), asfloat(buffer.Load3((offset + 32u))), asfloat(buffer.Load3((offset + 48u))));
}

[numthreads(1, 1, 1)]
void main() {
  tint_symbol_2(tint_symbol_1, 0u, tint_symbol_4(tint_symbol, 0u));
  return;
}
