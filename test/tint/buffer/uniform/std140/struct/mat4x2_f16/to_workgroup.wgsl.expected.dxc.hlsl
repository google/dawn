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
  vector<uint16_t, 2> v16 = vector<uint16_t, 2>(((uint2(v, v) >> uint2(0u, 16u)) & (65535u).xx));
  return asfloat16(v16);
}

matrix<float16_t, 4, 2> v_1(uint start_byte_offset) {
  vector<float16_t, 2> v_2 = tint_bitcast_to_f16(u[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)]);
  uint v_3 = (4u + start_byte_offset);
  vector<float16_t, 2> v_4 = tint_bitcast_to_f16(u[(v_3 / 16u)][((v_3 & 15u) >> 2u)]);
  uint v_5 = (8u + start_byte_offset);
  vector<float16_t, 2> v_6 = tint_bitcast_to_f16(u[(v_5 / 16u)][((v_5 & 15u) >> 2u)]);
  uint v_7 = (12u + start_byte_offset);
  return matrix<float16_t, 4, 2>(v_2, v_4, v_6, tint_bitcast_to_f16(u[(v_7 / 16u)][((v_7 & 15u) >> 2u)]));
}

S v_8(uint start_byte_offset) {
  int v_9 = asint(u[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)]);
  matrix<float16_t, 4, 2> v_10 = v_1((4u + start_byte_offset));
  uint v_11 = (64u + start_byte_offset);
  S v_12 = {v_9, v_10, asint(u[(v_11 / 16u)][((v_11 & 15u) >> 2u)])};
  return v_12;
}

typedef S ary_ret[4];
ary_ret v_13(uint start_byte_offset) {
  S a[4] = (S[4])0;
  {
    uint v_14 = 0u;
    v_14 = 0u;
    while(true) {
      uint v_15 = v_14;
      if ((v_15 >= 4u)) {
        break;
      }
      S v_16 = v_8((start_byte_offset + (v_15 * 128u)));
      a[v_15] = v_16;
      {
        v_14 = (v_15 + 1u);
      }
    }
  }
  S v_17[4] = a;
  return v_17;
}

void f_inner(uint tint_local_index) {
  {
    uint v_18 = 0u;
    v_18 = tint_local_index;
    while(true) {
      uint v_19 = v_18;
      if ((v_19 >= 4u)) {
        break;
      }
      S v_20 = (S)0;
      w[v_19] = v_20;
      {
        v_18 = (v_19 + 1u);
      }
    }
  }
  GroupMemoryBarrierWithGroupSync();
  S v_21[4] = v_13(0u);
  w = v_21;
  S v_22 = v_8(256u);
  w[int(1)] = v_22;
  w[int(3)].m = v_1(260u);
  w[int(1)].m[int(0)] = tint_bitcast_to_f16(u[0u].z).yx;
}

[numthreads(1, 1, 1)]
void f(f_inputs inputs) {
  f_inner(inputs.tint_local_index);
}

