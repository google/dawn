struct S {
  int before;
  float4x2 m;
  int after;
};

struct f_inputs {
  uint tint_local_index : SV_GroupIndex;
};


cbuffer cbuffer_u : register(b0) {
  uint4 u[32];
};
groupshared S w[4];
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

S v_11(uint start_byte_offset) {
  int v_12 = asint(u[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)]);
  float4x2 v_13 = v((8u + start_byte_offset));
  uint v_14 = (64u + start_byte_offset);
  S v_15 = {v_12, v_13, asint(u[(v_14 / 16u)][((v_14 & 15u) >> 2u)])};
  return v_15;
}

typedef S ary_ret[4];
ary_ret v_16(uint start_byte_offset) {
  S a[4] = (S[4])0;
  {
    uint idx = 0u;
    while(true) {
      uint v_17 = idx;
      if ((v_17 >= 4u)) {
        break;
      }
      S v_18 = v_11((start_byte_offset + (v_17 * 128u)));
      a[v_17] = v_18;
      {
        idx = (idx + 1u);
      }
    }
  }
  S v_19[4] = a;
  return v_19;
}

void f_inner(uint tint_local_index) {
  {
    uint idx = tint_local_index;
    while(true) {
      uint v_20 = idx;
      if ((v_20 >= 4u)) {
        break;
      }
      S v_21 = (S)0;
      w[v_20] = v_21;
      {
        idx = (idx + 1u);
      }
    }
  }
  GroupMemoryBarrierWithGroupSync();
  S v_22[4] = v_16(0u);
  w = v_22;
  S v_23 = v_11(256u);
  w[int(1)] = v_23;
  w[int(3)].m = v(264u);
  w[int(1)].m[int(0)] = asfloat(u[1u].xy).yx;
}

[numthreads(1, 1, 1)]
void f(f_inputs inputs) {
  f_inner(inputs.tint_local_index);
}

