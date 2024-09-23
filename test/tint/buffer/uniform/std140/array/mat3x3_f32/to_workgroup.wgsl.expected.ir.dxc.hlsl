struct f_inputs {
  uint tint_local_index : SV_GroupIndex;
};


cbuffer cbuffer_u : register(b0) {
  uint4 u[12];
};
groupshared float3x3 w[4];
float3x3 v(uint start_byte_offset) {
  float3 v_1 = asfloat(u[(start_byte_offset / 16u)].xyz);
  float3 v_2 = asfloat(u[((16u + start_byte_offset) / 16u)].xyz);
  return float3x3(v_1, v_2, asfloat(u[((32u + start_byte_offset) / 16u)].xyz));
}

typedef float3x3 ary_ret[4];
ary_ret v_3(uint start_byte_offset) {
  float3x3 a[4] = (float3x3[4])0;
  {
    uint v_4 = 0u;
    v_4 = 0u;
    while(true) {
      uint v_5 = v_4;
      if ((v_5 >= 4u)) {
        break;
      }
      a[v_5] = v((start_byte_offset + (v_5 * 48u)));
      {
        v_4 = (v_5 + 1u);
      }
      continue;
    }
  }
  float3x3 v_6[4] = a;
  return v_6;
}

void f_inner(uint tint_local_index) {
  {
    uint v_7 = 0u;
    v_7 = tint_local_index;
    while(true) {
      uint v_8 = v_7;
      if ((v_8 >= 4u)) {
        break;
      }
      w[v_8] = float3x3((0.0f).xxx, (0.0f).xxx, (0.0f).xxx);
      {
        v_7 = (v_8 + 1u);
      }
      continue;
    }
  }
  GroupMemoryBarrierWithGroupSync();
  float3x3 v_9[4] = v_3(0u);
  w = v_9;
  w[int(1)] = v(96u);
  w[int(1)][int(0)] = asfloat(u[1u].xyz).zxy;
  w[int(1)][int(0)][0u] = asfloat(u[1u].x);
}

[numthreads(1, 1, 1)]
void f(f_inputs inputs) {
  f_inner(inputs.tint_local_index);
}

