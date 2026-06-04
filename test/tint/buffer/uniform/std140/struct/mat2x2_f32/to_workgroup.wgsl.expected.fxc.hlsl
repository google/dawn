struct S {
  int before;
  float2x2 m;
  int after;
};

struct f_inputs {
  uint tint_local_index : SV_GroupIndex;
};


cbuffer cbuffer_u : register(b0) {
  uint4 u[32];
};
groupshared S w[4];
float2x2 v(uint start_byte_offset) {
  uint4 v_1 = u[(start_byte_offset / 16u)];
  float2 v_2 = asfloat((((((start_byte_offset & 15u) >> 2u) == 2u)) ? (v_1.zw) : (v_1.xy)));
  uint v_3 = (8u + start_byte_offset);
  uint4 v_4 = u[(v_3 / 16u)];
  return float2x2(v_2, asfloat((((((v_3 & 15u) >> 2u) == 2u)) ? (v_4.zw) : (v_4.xy))));
}

S v_5(uint start_byte_offset) {
  int v_6 = asint(u[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)]);
  float2x2 v_7 = v((8u + start_byte_offset));
  uint v_8 = (64u + start_byte_offset);
  S v_9 = {v_6, v_7, asint(u[(v_8 / 16u)][((v_8 & 15u) >> 2u)])};
  return v_9;
}

typedef S ary_ret[4];
ary_ret v_10(uint start_byte_offset) {
  S a[4] = (S[4])0;
  {
    uint v_11 = 0u;
    v_11 = 0u;
    while(true) {
      uint v_12 = v_11;
      if ((v_12 >= 4u)) {
        break;
      }
      S v_13 = v_5((start_byte_offset + (v_12 * 128u)));
      a[v_12] = v_13;
      {
        v_11 = (v_12 + 1u);
      }
    }
  }
  S v_14[4] = a;
  return v_14;
}

void f_inner(uint tint_local_index) {
  {
    uint v_15 = 0u;
    v_15 = tint_local_index;
    while(true) {
      uint v_16 = v_15;
      if ((v_16 >= 4u)) {
        break;
      }
      S v_17 = (S)0;
      w[v_16] = v_17;
      {
        v_15 = (v_16 + 1u);
      }
    }
  }
  GroupMemoryBarrierWithGroupSync();
  S v_18[4] = v_10(0u);
  w = v_18;
  S v_19 = v_5(256u);
  w[1u] = v_19;
  w[3u].m = v(264u);
  w[1u].m[0u] = asfloat(u[1u].xy).yx;
}

[numthreads(1, 1, 1)]
void f(f_inputs inputs) {
  f_inner(inputs.tint_local_index);
}

