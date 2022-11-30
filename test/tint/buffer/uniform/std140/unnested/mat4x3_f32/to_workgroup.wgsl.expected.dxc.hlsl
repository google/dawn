cbuffer cbuffer_u : register(b0, space0) {
  uint4 u[4];
};
groupshared float4x3 w;

struct tint_symbol_1 {
  uint local_invocation_index : SV_GroupIndex;
};

float4x3 tint_symbol_2(uint4 buffer[4], uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  const uint scalar_offset_1 = ((offset + 16u)) / 4;
  const uint scalar_offset_2 = ((offset + 32u)) / 4;
  const uint scalar_offset_3 = ((offset + 48u)) / 4;
  return float4x3(asfloat(buffer[scalar_offset / 4].xyz), asfloat(buffer[scalar_offset_1 / 4].xyz), asfloat(buffer[scalar_offset_2 / 4].xyz), asfloat(buffer[scalar_offset_3 / 4].xyz));
}

void f_inner(uint local_invocation_index) {
  {
    w = float4x3((0.0f).xxx, (0.0f).xxx, (0.0f).xxx, (0.0f).xxx);
  }
  GroupMemoryBarrierWithGroupSync();
  w = tint_symbol_2(u, 0u);
  w[1] = asfloat(u[0].xyz);
  w[1] = asfloat(u[0].xyz).zxy;
  w[0][1] = asfloat(u[1].x);
}

[numthreads(1, 1, 1)]
void f(tint_symbol_1 tint_symbol) {
  f_inner(tint_symbol.local_invocation_index);
  return;
}
