struct S {
  int before;
  matrix<float16_t, 4, 3> m;
  int after;
};

struct f_inputs {
  uint tint_local_index : SV_GroupIndex;
};


cbuffer cbuffer_u : register(b0) {
  uint4 u[32];
};
groupshared S w[4];
vector<float16_t, 4> tint_bitcast_to_f16(uint2 src) {
  uint2 v = src;
  vector<uint16_t, 4> v16 = vector<uint16_t, 4>(((v.xxyy >> uint4(0u, 16u, 0u, 16u)) & (65535u).xxxx));
  return asfloat16(v16);
}

matrix<float16_t, 4, 3> v_1(uint start_byte_offset) {
  uint4 v_2 = u[(start_byte_offset / 16u)];
  vector<float16_t, 3> v_3 = tint_bitcast_to_f16(select((((start_byte_offset & 15u) >> 2u) == 2u), v_2.zw, v_2.xy)).xyz;
  uint v_4 = (8u + start_byte_offset);
  uint4 v_5 = u[(v_4 / 16u)];
  vector<float16_t, 3> v_6 = tint_bitcast_to_f16(select((((v_4 & 15u) >> 2u) == 2u), v_5.zw, v_5.xy)).xyz;
  uint v_7 = (16u + start_byte_offset);
  uint4 v_8 = u[(v_7 / 16u)];
  vector<float16_t, 3> v_9 = tint_bitcast_to_f16(select((((v_7 & 15u) >> 2u) == 2u), v_8.zw, v_8.xy)).xyz;
  uint v_10 = (24u + start_byte_offset);
  uint4 v_11 = u[(v_10 / 16u)];
  return matrix<float16_t, 4, 3>(v_3, v_6, v_9, tint_bitcast_to_f16(select((((v_10 & 15u) >> 2u) == 2u), v_11.zw, v_11.xy)).xyz);
}

S v_12(uint start_byte_offset) {
  int v_13 = asint(u[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)]);
  matrix<float16_t, 4, 3> v_14 = v_1((8u + start_byte_offset));
  uint v_15 = (64u + start_byte_offset);
  S v_16 = {v_13, v_14, asint(u[(v_15 / 16u)][((v_15 & 15u) >> 2u)])};
  return v_16;
}

typedef S ary_ret[4];
ary_ret v_17(uint start_byte_offset) {
  S a[4] = (S[4])0;
  {
    uint v_18 = 0u;
    v_18 = 0u;
    while(true) {
      uint v_19 = v_18;
      if ((v_19 >= 4u)) {
        break;
      }
      S v_20 = v_12((start_byte_offset + (v_19 * 128u)));
      a[v_19] = v_20;
      {
        v_18 = (v_19 + 1u);
      }
    }
  }
  S v_21[4] = a;
  return v_21;
}

void f_inner(uint tint_local_index) {
  {
    uint v_22 = 0u;
    v_22 = tint_local_index;
    while(true) {
      uint v_23 = v_22;
      if ((v_23 >= 4u)) {
        break;
      }
      S v_24 = (S)0;
      w[v_23] = v_24;
      {
        v_22 = (v_23 + 1u);
      }
    }
  }
  GroupMemoryBarrierWithGroupSync();
  S v_25[4] = v_17(0u);
  w = v_25;
  S v_26 = v_12(256u);
  w[1u] = v_26;
  w[3u].m = v_1(264u);
  w[1u].m[0u] = tint_bitcast_to_f16(u[1u].xy).xyz.zxy;
}

[numthreads(1, 1, 1)]
void f(f_inputs inputs) {
  f_inner(inputs.tint_local_index);
}

