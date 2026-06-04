struct S {
  int before;
  matrix<float16_t, 4, 2> m;
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

matrix<float16_t, 4, 2> v_2(uint start_byte_offset) {
  vector<float16_t, 2> v_3 = tint_bitcast_to_f16(u[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)]);
  uint v_4 = (4u + start_byte_offset);
  vector<float16_t, 2> v_5 = tint_bitcast_to_f16(u[(v_4 / 16u)][((v_4 & 15u) >> 2u)]);
  uint v_6 = (8u + start_byte_offset);
  vector<float16_t, 2> v_7 = tint_bitcast_to_f16(u[(v_6 / 16u)][((v_6 & 15u) >> 2u)]);
  uint v_8 = (12u + start_byte_offset);
  return matrix<float16_t, 4, 2>(v_3, v_5, v_7, tint_bitcast_to_f16(u[(v_8 / 16u)][((v_8 & 15u) >> 2u)]));
}

S v_9(uint start_byte_offset) {
  int v_10 = asint(u[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)]);
  matrix<float16_t, 4, 2> v_11 = v_2((4u + start_byte_offset));
  uint v_12 = (64u + start_byte_offset);
  S v_13 = {v_10, v_11, asint(u[(v_12 / 16u)][((v_12 & 15u) >> 2u)])};
  return v_13;
}

typedef S ary_ret[4];
ary_ret v_14(uint start_byte_offset) {
  S a[4] = (S[4])0;
  {
    uint v_15 = 0u;
    v_15 = 0u;
    while(true) {
      uint v_16 = v_15;
      if ((v_16 >= 4u)) {
        break;
      }
      S v_17 = v_9((start_byte_offset + (v_16 * 128u)));
      a[v_16] = v_17;
      {
        v_15 = (v_16 + 1u);
      }
    }
  }
  S v_18[4] = a;
  return v_18;
}

void f_inner(uint tint_local_index) {
  {
    uint v_19 = 0u;
    v_19 = tint_local_index;
    while(true) {
      uint v_20 = v_19;
      if ((v_20 >= 4u)) {
        break;
      }
      S v_21 = (S)0;
      w[v_20] = v_21;
      {
        v_19 = (v_20 + 1u);
      }
    }
  }
  GroupMemoryBarrierWithGroupSync();
  S v_22[4] = v_14(0u);
  w = v_22;
  S v_23 = v_9(256u);
  w[1u] = v_23;
  w[3u].m = v_2(260u);
  w[1u].m[0u] = tint_bitcast_to_f16(u[0u].z).yx;
}

[numthreads(1, 1, 1)]
void f(f_inputs inputs) {
  f_inner(inputs.tint_local_index);
}

