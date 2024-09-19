struct f_inputs {
  uint tint_local_index : SV_GroupIndex;
};


cbuffer cbuffer_u : register(b0) {
  uint4 u[8];
};
groupshared float2x4 w[4];
float2x4 v(uint start_byte_offset) {
  float4 v_1 = asfloat(u[(start_byte_offset / 16u)]);
  return float2x4(v_1, asfloat(u[((16u + start_byte_offset) / 16u)]));
}

typedef float2x4 ary_ret[4];
ary_ret v_2(uint start_byte_offset) {
  float2x4 a[4] = (float2x4[4])0;
  {
    uint v_3 = 0u;
    v_3 = 0u;
    while(true) {
      uint v_4 = v_3;
      if ((v_4 >= 4u)) {
        break;
      }
      a[v_4] = v((start_byte_offset + (v_4 * 32u)));
      {
        v_3 = (v_4 + 1u);
      }
      continue;
    }
  }
  float2x4 v_5[4] = a;
  return v_5;
}

void f_inner(uint tint_local_index) {
  {
    uint v_6 = 0u;
    v_6 = tint_local_index;
    while(true) {
      uint v_7 = v_6;
      if ((v_7 >= 4u)) {
        break;
      }
      w[v_7] = float2x4((0.0f).xxxx, (0.0f).xxxx);
      {
        v_6 = (v_7 + 1u);
      }
      continue;
    }
  }
  GroupMemoryBarrierWithGroupSync();
  float2x4 v_8[4] = v_2(0u);
  w = v_8;
  w[int(1)] = v(64u);
  w[int(1)][int(0)] = asfloat(u[1u]).ywxz;
  w[int(1)][int(0)][0u] = asfloat(u[1u].x);
}

[numthreads(1, 1, 1)]
void f(f_inputs inputs) {
  f_inner(inputs.tint_local_index);
}

