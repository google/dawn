struct S {
  int before;
  matrix<float16_t, 3, 2> m;
  int after;
};

struct f_inputs {
  uint tint_local_index : SV_GroupIndex;
};


cbuffer cbuffer_u : register(b0) {
  uint4 u[32];
};
groupshared S w[4];
vector<float16_t, 2> tint_bitcast_to_f16(uint src) {
  uint v = src;
  uint2 v_1 = uint2(v, v);
  vector<uint16_t, 2> v16 = vector<uint16_t, 2>(((v_1 >> uint2(0u, 16u)) & (65535u).xx));
  return asfloat16(v16);
}

matrix<float16_t, 3, 2> v_2(uint start_byte_offset) {
  vector<float16_t, 2> v_3 = tint_bitcast_to_f16(u[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)]);
  uint v_4 = (4u + start_byte_offset);
  vector<float16_t, 2> v_5 = tint_bitcast_to_f16(u[(v_4 / 16u)][((v_4 & 15u) >> 2u)]);
  uint v_6 = (8u + start_byte_offset);
  return matrix<float16_t, 3, 2>(v_3, v_5, tint_bitcast_to_f16(u[(v_6 / 16u)][((v_6 & 15u) >> 2u)]));
}

S v_7(uint start_byte_offset) {
  int v_8 = asint(u[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)]);
  matrix<float16_t, 3, 2> v_9 = v_2((4u + start_byte_offset));
  uint v_10 = (64u + start_byte_offset);
  S v_11 = {v_8, v_9, asint(u[(v_10 / 16u)][((v_10 & 15u) >> 2u)])};
  return v_11;
}

typedef S ary_ret[4];
ary_ret v_12(uint start_byte_offset) {
  S a[4] = (S[4])0;
  {
    uint v_13 = 0u;
    v_13 = 0u;
    while(true) {
      uint v_14 = v_13;
      if ((v_14 >= 4u)) {
        break;
      }
      S v_15 = v_7((start_byte_offset + (v_14 * 128u)));
      a[v_14] = v_15;
      {
        v_13 = (v_14 + 1u);
      }
    }
  }
  S v_16[4] = a;
  return v_16;
}

void f_inner(uint tint_local_index) {
  {
    uint v_17 = 0u;
    v_17 = tint_local_index;
    while(true) {
      uint v_18 = v_17;
      if ((v_18 >= 4u)) {
        break;
      }
      S v_19 = (S)0;
      w[v_18] = v_19;
      {
        v_17 = (v_18 + 1u);
      }
    }
  }
  GroupMemoryBarrierWithGroupSync();
  S v_20[4] = v_12(0u);
  w = v_20;
  S v_21 = v_7(256u);
  w[1u] = v_21;
  w[3u].m = v_2(260u);
  w[1u].m[0u] = tint_bitcast_to_f16(u[0u].z).yx;
}

[numthreads(1, 1, 1)]
void f(f_inputs inputs) {
  f_inner(inputs.tint_local_index);
}

