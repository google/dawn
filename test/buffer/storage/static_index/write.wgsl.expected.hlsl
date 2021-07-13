struct Inner {
  int x;
};
struct tint_padded_array_element {
  Inner el;
};

void tint_symbol_6(RWByteAddressBuffer buffer, uint offset, float2x3 value) {
  buffer.Store3((offset + 0u), asuint(value[0u]));
  buffer.Store3((offset + 16u), asuint(value[1u]));
}

void tint_symbol_8(RWByteAddressBuffer buffer, uint offset, float3x2 value) {
  buffer.Store2((offset + 0u), asuint(value[0u]));
  buffer.Store2((offset + 8u), asuint(value[1u]));
  buffer.Store2((offset + 16u), asuint(value[2u]));
}

void tint_symbol_9(RWByteAddressBuffer buffer, uint offset, Inner value) {
  buffer.Store((offset + 0u), asuint(value.x));
}

void tint_symbol_10(RWByteAddressBuffer buffer, uint offset, tint_padded_array_element value[4]) {
  tint_symbol_9(buffer, (offset + 0u), value[0u].el);
  tint_symbol_9(buffer, (offset + 16u), value[1u].el);
  tint_symbol_9(buffer, (offset + 32u), value[2u].el);
  tint_symbol_9(buffer, (offset + 48u), value[3u].el);
}

RWByteAddressBuffer s : register(u0, space0);

[numthreads(1, 1, 1)]
void main() {
  s.Store3(0u, asuint(int3(0, 0, 0)));
  s.Store(12u, asuint(0));
  s.Store3(16u, asuint(uint3(0u, 0u, 0u)));
  s.Store(28u, asuint(0u));
  s.Store3(32u, asuint(float3(0.0f, 0.0f, 0.0f)));
  s.Store(44u, asuint(0.0f));
  tint_symbol_6(s, 48u, float2x3(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f));
  tint_symbol_8(s, 80u, float3x2(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f));
  const Inner tint_symbol_11 = (Inner)0;
  tint_symbol_9(s, 104u, tint_symbol_11);
  const tint_padded_array_element tint_symbol_12[4] = (tint_padded_array_element[4])0;
  tint_symbol_10(s, 108u, tint_symbol_12);
  return;
}
