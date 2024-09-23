struct f_inputs {
  uint tint_local_index : SV_GroupIndex;
};


cbuffer cbuffer_u : register(b0) {
  uint4 u[2];
};
groupshared float2x4 w;
float2x4 v(uint start_byte_offset) {
  float4 v_1 = asfloat(u[(start_byte_offset / 16u)]);
  return float2x4(v_1, asfloat(u[((16u + start_byte_offset) / 16u)]));
}

void f_inner(uint tint_local_index) {
  if ((tint_local_index == 0u)) {
    w = float2x4((0.0f).xxxx, (0.0f).xxxx);
  }
  GroupMemoryBarrierWithGroupSync();
  w = v(0u);
  w[int(1)] = asfloat(u[0u]);
  w[int(1)] = asfloat(u[0u]).ywxz;
  w[int(0)][int(1)] = asfloat(u[1u].x);
}

[numthreads(1, 1, 1)]
void f(f_inputs inputs) {
  f_inner(inputs.tint_local_index);
}

