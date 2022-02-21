RWByteAddressBuffer ssbo : register(u0, space0);

float2x2 tint_symbol(RWByteAddressBuffer buffer, uint offset) {
  return float2x2(asfloat(buffer.Load2((offset + 0u))), asfloat(buffer.Load2((offset + 8u))));
}

void tint_symbol_2(RWByteAddressBuffer buffer, uint offset, float2x2 value) {
  buffer.Store2((offset + 0u), asuint(value[0u]));
  buffer.Store2((offset + 8u), asuint(value[1u]));
}

[numthreads(1, 1, 1)]
void f() {
  const float2x2 v = tint_symbol(ssbo, 0u);
  tint_symbol_2(ssbo, 0u, v);
  return;
}
