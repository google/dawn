struct f_inputs {
  uint tint_local_index : SV_GroupIndex;
};


cbuffer cbuffer_u : register(b0) {
  uint4 u[1];
};
groupshared float2x2 w;
float2x2 v(uint start_byte_offset) {
  uint4 v_1 = u[(start_byte_offset / 16u)];
  uint v_2 = (8u + start_byte_offset);
  uint4 v_3 = u[(v_2 / 16u)];
  return float2x2(asfloat(select((((start_byte_offset & 15u) >> 2u) == 2u), v_1.zw, v_1.xy)), asfloat(select((((v_2 & 15u) >> 2u) == 2u), v_3.zw, v_3.xy)));
}

void f_inner(uint tint_local_index) {
  if ((tint_local_index < 1u)) {
    w = float2x2((0.0f).xx, (0.0f).xx);
  }
  GroupMemoryBarrierWithGroupSync();
  w = v(0u);
  w[1u] = asfloat(u[0u].xy);
  w[1u] = asfloat(u[0u].xy).yx;
  w[0u].y = asfloat(u[0u].z);
}

[numthreads(1, 1, 1)]
void f(f_inputs inputs) {
  f_inner(inputs.tint_local_index);
}

