struct f_inputs {
  uint tint_local_index : SV_GroupIndex;
};


cbuffer cbuffer_u : register(b0) {
  uint4 u[8];
};
groupshared float4x2 w[4];
float4x2 v(uint start_byte_offset) {
  uint4 v_1 = u[(start_byte_offset / 16u)];
  float2 v_2 = asfloat((((((start_byte_offset & 15u) >> 2u) == 2u)) ? (v_1.zw) : (v_1.xy)));
  uint v_3 = (8u + start_byte_offset);
  uint4 v_4 = u[(v_3 / 16u)];
  float2 v_5 = asfloat((((((v_3 & 15u) >> 2u) == 2u)) ? (v_4.zw) : (v_4.xy)));
  uint v_6 = (16u + start_byte_offset);
  uint4 v_7 = u[(v_6 / 16u)];
  float2 v_8 = asfloat((((((v_6 & 15u) >> 2u) == 2u)) ? (v_7.zw) : (v_7.xy)));
  uint v_9 = (24u + start_byte_offset);
  uint4 v_10 = u[(v_9 / 16u)];
  return float4x2(v_2, v_5, v_8, asfloat((((((v_9 & 15u) >> 2u) == 2u)) ? (v_10.zw) : (v_10.xy))));
}

typedef float4x2 ary_ret[4];
ary_ret v_11(uint start_byte_offset) {
  float4x2 a[4] = (float4x2[4])0;
  {
    uint idx = 0u;
    while(true) {
      uint v_12 = idx;
      if ((v_12 >= 4u)) {
        break;
      }
      a[v_12] = v((start_byte_offset + (v_12 * 32u)));
      {
        idx = (idx + 1u);
      }
    }
  }
  float4x2 v_13[4] = a;
  return v_13;
}

void f_inner(uint tint_local_index) {
  {
    uint idx = tint_local_index;
    while(true) {
      uint v_14 = idx;
      if ((v_14 >= 4u)) {
        break;
      }
      w[v_14] = float4x2((0.0f).xx, (0.0f).xx, (0.0f).xx, (0.0f).xx);
      {
        idx = (idx + 1u);
      }
    }
  }
  GroupMemoryBarrierWithGroupSync();
  float4x2 v_15[4] = v_11(0u);
  w = v_15;
  w[int(1)] = v(64u);
  w[int(1)][int(0)] = asfloat(u[0u].zw).yx;
  w[int(1)][int(0)].x = asfloat(u[0u].z);
}

[numthreads(1, 1, 1)]
void f(f_inputs inputs) {
  f_inner(inputs.tint_local_index);
}

