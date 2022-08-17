struct S {
  float f;
};

ByteAddressBuffer tint_symbol : register(t0, space0);
RWByteAddressBuffer tint_symbol_1 : register(u1, space0);

void tint_symbol_2(RWByteAddressBuffer buffer, uint offset, S value) {
  buffer.Store((offset + 0u), asuint(value.f));
}

S tint_symbol_4(ByteAddressBuffer buffer, uint offset) {
  const S tint_symbol_6 = {asfloat(buffer.Load((offset + 0u)))};
  return tint_symbol_6;
}

[numthreads(1, 1, 1)]
void main() {
  tint_symbol_2(tint_symbol_1, 0u, tint_symbol_4(tint_symbol, 0u));
  return;
}
