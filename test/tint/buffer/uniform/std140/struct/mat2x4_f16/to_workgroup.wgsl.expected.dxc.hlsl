struct S {
  int before;
  matrix<float16_t, 2, 4> m;
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

matrix<float16_t, 2, 4> v_1(uint start_byte_offset) {
  uint4 v_2 = u[(start_byte_offset / 16u)];
  vector<float16_t, 4> v_3 = tint_bitcast_to_f16(select((((start_byte_offset & 15u) >> 2u) == 2u), v_2.zw, v_2.xy));
  uint v_4 = (8u + start_byte_offset);
  uint4 v_5 = u[(v_4 / 16u)];
  return matrix<float16_t, 2, 4>(v_3, tint_bitcast_to_f16(select((((v_4 & 15u) >> 2u) == 2u), v_5.zw, v_5.xy)));
}

S v_6(uint start_byte_offset) {
  int v_7 = asint(u[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)]);
  matrix<float16_t, 2, 4> v_8 = v_1((8u + start_byte_offset));
  uint v_9 = (64u + start_byte_offset);
  S v_10 = {v_7, v_8, asint(u[(v_9 / 16u)][((v_9 & 15u) >> 2u)])};
  return v_10;
}

typedef S ary_ret[4];
ary_ret v_11(uint start_byte_offset) {
  S a[4] = (S[4])0;
  {
    uint idx = 0u;
    while(true) {
      uint v_12 = idx;
      if ((v_12 >= 4u)) {
        break;
      }
      S v_13 = v_6((start_byte_offset + (v_12 * 128u)));
      a[v_12] = v_13;
      {
        idx = (idx + 1u);
      }
    }
  }
  S v_14[4] = a;
  return v_14;
}

void f_inner(uint tint_local_index) {
  {
    uint idx = tint_local_index;
    while(true) {
      uint v_15 = idx;
      if ((v_15 >= 4u)) {
        break;
      }
      S v_16 = (S)0;
      w[v_15] = v_16;
      {
        idx = (idx + 1u);
      }
    }
  }
  GroupMemoryBarrierWithGroupSync();
  S v_17[4] = v_11(0u);
  w = v_17;
  S v_18 = v_6(256u);
  w[int(1)] = v_18;
  w[int(3)].m = v_1(264u);
  w[int(1)].m[int(0)] = tint_bitcast_to_f16(u[1u].xy).ywxz;
}

[numthreads(1, 1, 1)]
void f(f_inputs inputs) {
  f_inner(inputs.tint_local_index);
}

