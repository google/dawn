void tint_symbol_8(RWByteAddressBuffer buffer, uint offset, float2x3 value) {
  buffer.Store3((offset + 0u), asuint(value[0u]));
  buffer.Store3((offset + 16u), asuint(value[1u]));
}

void tint_symbol_10(RWByteAddressBuffer buffer, uint offset, float3x2 value) {
  buffer.Store2((offset + 0u), asuint(value[0u]));
  buffer.Store2((offset + 8u), asuint(value[1u]));
  buffer.Store2((offset + 16u), asuint(value[2u]));
}

void tint_symbol_12(RWByteAddressBuffer buffer, uint offset, int4 value[4]) {
  buffer.Store4((offset + 0u), asuint(value[0u]));
  buffer.Store4((offset + 16u), asuint(value[1u]));
  buffer.Store4((offset + 32u), asuint(value[2u]));
  buffer.Store4((offset + 48u), asuint(value[3u]));
}

RWByteAddressBuffer s : register(u0, space0);

struct tint_symbol_1 {
  uint idx : SV_GroupIndex;
};

[numthreads(1, 1, 1)]
void main(tint_symbol_1 tint_symbol) {
  const uint idx = tint_symbol.idx;
  s.Store3((176u * idx), asuint(int3(0, 0, 0)));
  s.Store(((176u * idx) + 12u), asuint(0));
  s.Store3(((176u * idx) + 16u), asuint(uint3(0u, 0u, 0u)));
  s.Store(((176u * idx) + 28u), asuint(0u));
  s.Store3(((176u * idx) + 32u), asuint(float3(0.0f, 0.0f, 0.0f)));
  s.Store(((176u * idx) + 44u), asuint(0.0f));
  tint_symbol_8(s, ((176u * idx) + 48u), float2x3(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f));
  tint_symbol_10(s, ((176u * idx) + 80u), float3x2(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f));
  const int4 tint_symbol_13[4] = (int4[4])0;
  tint_symbol_12(s, ((176u * idx) + 112u), tint_symbol_13);
  return;
}
