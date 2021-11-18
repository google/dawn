struct Inner {
  int x;
};
struct tint_padded_array_element {
  Inner el;
};

cbuffer cbuffer_s : register(b0, space0) {
  uint4 s[13];
};

float2x3 tint_symbol_7(uint4 buffer[13], uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  const uint scalar_offset_1 = ((offset + 16u)) / 4;
  return float2x3(asfloat(buffer[scalar_offset / 4].xyz), asfloat(buffer[scalar_offset_1 / 4].xyz));
}

float3x2 tint_symbol_8(uint4 buffer[13], uint offset) {
  const uint scalar_offset_2 = ((offset + 0u)) / 4;
  uint4 ubo_load = buffer[scalar_offset_2 / 4];
  const uint scalar_offset_3 = ((offset + 8u)) / 4;
  uint4 ubo_load_1 = buffer[scalar_offset_3 / 4];
  const uint scalar_offset_4 = ((offset + 16u)) / 4;
  uint4 ubo_load_2 = buffer[scalar_offset_4 / 4];
  return float3x2(asfloat(((scalar_offset_2 & 2) ? ubo_load.zw : ubo_load.xy)), asfloat(((scalar_offset_3 & 2) ? ubo_load_1.zw : ubo_load_1.xy)), asfloat(((scalar_offset_4 & 2) ? ubo_load_2.zw : ubo_load_2.xy)));
}

Inner tint_symbol_10(uint4 buffer[13], uint offset) {
  const uint scalar_offset_5 = ((offset + 0u)) / 4;
  const Inner tint_symbol_12 = {asint(buffer[scalar_offset_5 / 4][scalar_offset_5 % 4])};
  return tint_symbol_12;
}

typedef tint_padded_array_element tint_symbol_11_ret[4];
tint_symbol_11_ret tint_symbol_11(uint4 buffer[13], uint offset) {
  tint_padded_array_element arr[4] = (tint_padded_array_element[4])0;
  {
    [loop] for(uint i_1 = 0u; (i_1 < 4u); i_1 = (i_1 + 1u)) {
      arr[i_1].el = tint_symbol_10(buffer, (offset + (i_1 * 16u)));
    }
  }
  return arr;
}

[numthreads(1, 1, 1)]
void main() {
  const int3 a = asint(s[0].xyz);
  const int b = asint(s[0].w);
  const uint3 c = s[1].xyz;
  const uint d = s[1].w;
  const float3 e = asfloat(s[2].xyz);
  const float f = asfloat(s[2].w);
  const int2 g = asint(s[3].xy);
  const int2 h = asint(s[3].zw);
  const float2x3 i = tint_symbol_7(s, 64u);
  const float3x2 j = tint_symbol_8(s, 96u);
  const Inner k = tint_symbol_10(s, 128u);
  const tint_padded_array_element l[4] = tint_symbol_11(s, 144u);
  return;
}
