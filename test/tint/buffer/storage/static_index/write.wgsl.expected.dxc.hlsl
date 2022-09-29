struct Inner {
  int x;
};

RWByteAddressBuffer s : register(u0, space0);

void tint_symbol_6(RWByteAddressBuffer buffer, uint offset, float2x3 value) {
  buffer.Store3((offset + 0u), asuint(value[0u]));
  buffer.Store3((offset + 16u), asuint(value[1u]));
}

void tint_symbol_7(RWByteAddressBuffer buffer, uint offset, float3x2 value) {
  buffer.Store2((offset + 0u), asuint(value[0u]));
  buffer.Store2((offset + 8u), asuint(value[1u]));
  buffer.Store2((offset + 16u), asuint(value[2u]));
}

void tint_symbol_9(RWByteAddressBuffer buffer, uint offset, Inner value) {
  buffer.Store((offset + 0u), asuint(value.x));
}

void tint_symbol_10(RWByteAddressBuffer buffer, uint offset, Inner value[4]) {
  Inner array[4] = value;
  {
    for(uint i_1 = 0u; (i_1 < 4u); i_1 = (i_1 + 1u)) {
      tint_symbol_9(buffer, (offset + (i_1 * 4u)), array[i_1]);
    }
  }
}

[numthreads(1, 1, 1)]
void main() {
  s.Store3(0u, asuint((0).xxx));
  s.Store(12u, asuint(0));
  s.Store3(16u, asuint((0u).xxx));
  s.Store(28u, asuint(0u));
  s.Store3(32u, asuint((0.0f).xxx));
  s.Store(44u, asuint(0.0f));
  tint_symbol_6(s, 48u, float2x3((0.0f).xxx, (0.0f).xxx));
  tint_symbol_7(s, 80u, float3x2((0.0f).xx, (0.0f).xx, (0.0f).xx));
  const Inner tint_symbol_11 = (Inner)0;
  tint_symbol_9(s, 104u, tint_symbol_11);
  const Inner tint_symbol_12[4] = (Inner[4])0;
  tint_symbol_10(s, 108u, tint_symbol_12);
  return;
}
