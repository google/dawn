cbuffer cbuffer_u : register(b0) {
  uint4 u[2];
};
groupshared float2x4 w;

struct tint_symbol_1 {
  uint local_invocation_index : SV_GroupIndex;
};

float2x4 u_load(uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  const uint scalar_offset_1 = ((offset + 16u)) / 4;
  return float2x4(asfloat(u[scalar_offset / 4]), asfloat(u[scalar_offset_1 / 4]));
}

void f_inner(uint local_invocation_index) {
  {
    w = float2x4((0.0f).xxxx, (0.0f).xxxx);
  }
  GroupMemoryBarrierWithGroupSync();
  w = u_load(0u);
  w[1] = asfloat(u[0]);
  w[1] = asfloat(u[0]).ywxz;
  w[0][1] = asfloat(u[1].x);
}

[numthreads(1, 1, 1)]
void f(tint_symbol_1 tint_symbol) {
  f_inner(tint_symbol.local_invocation_index);
  return;
}
