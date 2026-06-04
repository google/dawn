struct S {
  int before;
  float3x2 m;
  int after;
};

struct f_inputs {
  uint tint_local_index : SV_GroupIndex;
};


cbuffer cbuffer_u : register(b0) {
  uint4 u[32];
};
groupshared S w[4];
float3x2 v(uint start_byte_offset) {
  uint4 v_1 = u[(start_byte_offset / 16u)];
  uint v_2 = (8u + start_byte_offset);
  uint4 v_3 = u[(v_2 / 16u)];
  uint v_4 = (16u + start_byte_offset);
  uint4 v_5 = u[(v_4 / 16u)];
  return float3x2(asfloat(select((((start_byte_offset & 15u) >> 2u) == 2u), v_1.zw, v_1.xy)), asfloat(select((((v_2 & 15u) >> 2u) == 2u), v_3.zw, v_3.xy)), asfloat(select((((v_4 & 15u) >> 2u) == 2u), v_5.zw, v_5.xy)));
}

S v_6(uint start_byte_offset) {
  int v_7 = asint(u[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)]);
  float3x2 v_8 = v((8u + start_byte_offset));
  uint v_9 = (64u + start_byte_offset);
  S v_10 = {v_7, v_8, asint(u[(v_9 / 16u)][((v_9 & 15u) >> 2u)])};
  return v_10;
}

typedef S ary_ret[4];
ary_ret v_11(uint start_byte_offset) {
  S a[4] = (S[4])0;
  {
    uint v_12 = 0u;
    v_12 = 0u;
    while(true) {
      uint v_13 = v_12;
      if ((v_13 >= 4u)) {
        break;
      }
      S v_14 = v_6((start_byte_offset + (v_13 * 128u)));
      a[v_13] = v_14;
      {
        v_12 = (v_13 + 1u);
      }
    }
  }
  S v_15[4] = a;
  return v_15;
}

void f_inner(uint tint_local_index) {
  {
    uint v_16 = 0u;
    v_16 = tint_local_index;
    while(true) {
      uint v_17 = v_16;
      if ((v_17 >= 4u)) {
        break;
      }
      S v_18 = (S)0;
      w[v_17] = v_18;
      {
        v_16 = (v_17 + 1u);
      }
    }
  }
  GroupMemoryBarrierWithGroupSync();
  S v_19[4] = v_11(0u);
  w = v_19;
  S v_20 = v_6(256u);
  w[1u] = v_20;
  w[3u].m = v(264u);
  w[1u].m[0u] = asfloat(u[1u].xy).yx;
}

[numthreads(1, 1, 1)]
void f(f_inputs inputs) {
  f_inner(inputs.tint_local_index);
}

