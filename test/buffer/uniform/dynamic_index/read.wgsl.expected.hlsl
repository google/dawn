cbuffer cbuffer_s : register(b0, space0) {
  uint4 s[96];
};

struct tint_symbol_1 {
  uint idx : SV_GroupIndex;
};

float2x3 tint_symbol_9(uint4 buffer[96], uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  const uint scalar_offset_1 = ((offset + 16u)) / 4;
  return float2x3(asfloat(buffer[scalar_offset / 4].xyz), asfloat(buffer[scalar_offset_1 / 4].xyz));
}

void main_inner(uint idx) {
  const uint scalar_offset_2 = ((192u * idx)) / 4;
  const int3 a = asint(s[scalar_offset_2 / 4].xyz);
  const uint scalar_offset_3 = (((192u * idx) + 12u)) / 4;
  const int b = asint(s[scalar_offset_3 / 4][scalar_offset_3 % 4]);
  const uint scalar_offset_4 = (((192u * idx) + 16u)) / 4;
  const uint3 c = s[scalar_offset_4 / 4].xyz;
  const uint scalar_offset_5 = (((192u * idx) + 28u)) / 4;
  const uint d = s[scalar_offset_5 / 4][scalar_offset_5 % 4];
  const uint scalar_offset_6 = (((192u * idx) + 32u)) / 4;
  const float3 e = asfloat(s[scalar_offset_6 / 4].xyz);
  const uint scalar_offset_7 = (((192u * idx) + 44u)) / 4;
  const float f = asfloat(s[scalar_offset_7 / 4][scalar_offset_7 % 4]);
  const uint scalar_offset_8 = (((192u * idx) + 48u)) / 4;
  uint4 ubo_load = s[scalar_offset_8 / 4];
  const int2 g = asint(((scalar_offset_8 & 2) ? ubo_load.zw : ubo_load.xy));
  const uint scalar_offset_9 = (((192u * idx) + 56u)) / 4;
  uint4 ubo_load_1 = s[scalar_offset_9 / 4];
  const int2 h = asint(((scalar_offset_9 & 2) ? ubo_load_1.zw : ubo_load_1.xy));
  const float2x3 i = tint_symbol_9(s, ((192u * idx) + 64u));
}

[numthreads(1, 1, 1)]
void main(tint_symbol_1 tint_symbol) {
  main_inner(tint_symbol.idx);
  return;
}
