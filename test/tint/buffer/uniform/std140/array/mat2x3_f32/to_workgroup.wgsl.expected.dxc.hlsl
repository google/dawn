struct f_inputs {
  uint tint_local_index : SV_GroupIndex;
};


cbuffer cbuffer_u : register(b0) {
  uint4 u[8];
};
groupshared float2x3 w[4];
float2x3 v(uint start_byte_offset) {
  return float2x3(asfloat(u[(start_byte_offset / 16u)].xyz), asfloat(u[((16u + start_byte_offset) / 16u)].xyz));
}

typedef float2x3 ary_ret[4];
ary_ret v_1(uint start_byte_offset) {
  float2x3 a[4] = (float2x3[4])0;
  {
    uint idx = 0u;
    while(true) {
      uint v_2 = idx;
      if ((v_2 >= 4u)) {
        break;
      }
      a[v_2] = v((start_byte_offset + (v_2 * 32u)));
      {
        idx = (idx + 1u);
      }
    }
  }
  float2x3 v_3[4] = a;
  return v_3;
}

void f_inner(uint tint_local_index) {
  {
    uint idx = tint_local_index;
    while(true) {
      uint v_4 = idx;
      if ((v_4 >= 4u)) {
        break;
      }
      w[v_4] = float2x3((0.0f).xxx, (0.0f).xxx);
      {
        idx = (idx + 1u);
      }
    }
  }
  GroupMemoryBarrierWithGroupSync();
  float2x3 v_5[4] = v_1(0u);
  w = v_5;
  w[int(1)] = v(64u);
  w[int(1)][int(0)] = asfloat(u[1u].xyz).zxy;
  w[int(1)][int(0)].x = asfloat(u[1u].x);
}

[numthreads(1, 1, 1)]
void f(f_inputs inputs) {
  f_inner(inputs.tint_local_index);
}

