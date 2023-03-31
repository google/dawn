cbuffer cbuffer_u : register(b0) {
  uint4 u[4];
};
groupshared float2x2 w[4];

struct tint_symbol_1 {
  uint local_invocation_index : SV_GroupIndex;
};

float2x2 u_load_1(uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  uint4 ubo_load = u[scalar_offset / 4];
  const uint scalar_offset_1 = ((offset + 8u)) / 4;
  uint4 ubo_load_1 = u[scalar_offset_1 / 4];
  return float2x2(asfloat(((scalar_offset & 2) ? ubo_load.zw : ubo_load.xy)), asfloat(((scalar_offset_1 & 2) ? ubo_load_1.zw : ubo_load_1.xy)));
}

typedef float2x2 u_load_ret[4];
u_load_ret u_load(uint offset) {
  float2x2 arr[4] = (float2x2[4])0;
  {
    for(uint i_1 = 0u; (i_1 < 4u); i_1 = (i_1 + 1u)) {
      arr[i_1] = u_load_1((offset + (i_1 * 16u)));
    }
  }
  return arr;
}

void f_inner(uint local_invocation_index) {
  {
    for(uint idx = local_invocation_index; (idx < 4u); idx = (idx + 1u)) {
      const uint i = idx;
      w[i] = float2x2((0.0f).xx, (0.0f).xx);
    }
  }
  GroupMemoryBarrierWithGroupSync();
  w = u_load(0u);
  w[1] = u_load_1(32u);
  w[1][0] = asfloat(u[0].zw).yx;
  w[1][0].x = asfloat(u[0].z);
}

[numthreads(1, 1, 1)]
void f(tint_symbol_1 tint_symbol) {
  f_inner(tint_symbol.local_invocation_index);
  return;
}
