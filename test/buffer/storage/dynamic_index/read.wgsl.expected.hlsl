ByteAddressBuffer s : register(t0, space0);

struct tint_symbol_1 {
  uint idx : SV_GroupIndex;
};

float2x3 tint_symbol_8(ByteAddressBuffer buffer, uint offset) {
  return float2x3(asfloat(buffer.Load3((offset + 0u))), asfloat(buffer.Load3((offset + 16u))));
}

float3x2 tint_symbol_9(ByteAddressBuffer buffer, uint offset) {
  return float3x2(asfloat(buffer.Load2((offset + 0u))), asfloat(buffer.Load2((offset + 8u))), asfloat(buffer.Load2((offset + 16u))));
}

typedef int4 tint_symbol_11_ret[4];
tint_symbol_11_ret tint_symbol_11(ByteAddressBuffer buffer, uint offset) {
  int4 arr_1[4] = (int4[4])0;
  {
    [loop] for(uint i_1 = 0u; (i_1 < 4u); i_1 = (i_1 + 1u)) {
      arr_1[i_1] = asint(buffer.Load4((offset + (i_1 * 16u))));
    }
  }
  return arr_1;
}

void main_inner(uint idx) {
  const int3 a = asint(s.Load3((176u * idx)));
  const int b = asint(s.Load(((176u * idx) + 12u)));
  const uint3 c = s.Load3(((176u * idx) + 16u));
  const uint d = s.Load(((176u * idx) + 28u));
  const float3 e = asfloat(s.Load3(((176u * idx) + 32u)));
  const float f = asfloat(s.Load(((176u * idx) + 44u)));
  const float2x3 g = tint_symbol_8(s, ((176u * idx) + 48u));
  const float3x2 h = tint_symbol_9(s, ((176u * idx) + 80u));
  const int4 i[4] = tint_symbol_11(s, ((176u * idx) + 112u));
}

[numthreads(1, 1, 1)]
void main(tint_symbol_1 tint_symbol) {
  main_inner(tint_symbol.idx);
  return;
}
