struct Inner {
  float f;
};
struct S {
  Inner inner;
};

ByteAddressBuffer tint_symbol : register(t0, space0);
RWByteAddressBuffer tint_symbol_1 : register(u1, space0);

void tint_symbol_3(RWByteAddressBuffer buffer, uint offset, Inner value) {
  buffer.Store((offset + 0u), asuint(value.f));
}

void tint_symbol_2(RWByteAddressBuffer buffer, uint offset, S value) {
  tint_symbol_3(buffer, (offset + 0u), value.inner);
}

Inner tint_symbol_6(ByteAddressBuffer buffer, uint offset) {
  const Inner tint_symbol_8 = {asfloat(buffer.Load((offset + 0u)))};
  return tint_symbol_8;
}

S tint_symbol_5(ByteAddressBuffer buffer, uint offset) {
  const S tint_symbol_9 = {tint_symbol_6(buffer, (offset + 0u))};
  return tint_symbol_9;
}

[numthreads(1, 1, 1)]
void main() {
  tint_symbol_2(tint_symbol_1, 0u, tint_symbol_5(tint_symbol, 0u));
  return;
}
