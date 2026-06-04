struct S {
  int before;
  matrix<float16_t, 2, 2> m;
  int after;
};


cbuffer cbuffer_u : register(b0) {
  uint4 u[32];
};
static S p[4] = (S[4])0;
vector<float16_t, 2> tint_bitcast_to_f16(uint src) {
  uint v = src;
  uint2 v_1 = uint2(v, v);
  vector<uint16_t, 2> v16 = vector<uint16_t, 2>(((v_1 >> uint2(0u, 16u)) & (65535u).xx));
  return asfloat16(v16);
}

matrix<float16_t, 2, 2> v_2(uint start_byte_offset) {
  vector<float16_t, 2> v_3 = tint_bitcast_to_f16(u[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)]);
  uint v_4 = (4u + start_byte_offset);
  return matrix<float16_t, 2, 2>(v_3, tint_bitcast_to_f16(u[(v_4 / 16u)][((v_4 & 15u) >> 2u)]));
}

S v_5(uint start_byte_offset) {
  int v_6 = asint(u[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)]);
  matrix<float16_t, 2, 2> v_7 = v_2((4u + start_byte_offset));
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

[numthreads(1, 1, 1)]
void f() {
  S v_15[4] = v_10(0u);
  p = v_15;
  S v_16 = v_5(256u);
  p[1u] = v_16;
  p[3u].m = v_2(260u);
  p[1u].m[0u] = tint_bitcast_to_f16(u[0u].z).yx;
}

