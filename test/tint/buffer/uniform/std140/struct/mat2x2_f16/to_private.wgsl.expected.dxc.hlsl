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
  vector<uint16_t, 2> v16 = vector<uint16_t, 2>(((uint2(v, v) >> uint2(0u, 16u)) & (65535u).xx));
  return asfloat16(v16);
}

matrix<float16_t, 2, 2> v_1(uint start_byte_offset) {
  vector<float16_t, 2> v_2 = tint_bitcast_to_f16(u[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)]);
  uint v_3 = (4u + start_byte_offset);
  return matrix<float16_t, 2, 2>(v_2, tint_bitcast_to_f16(u[(v_3 / 16u)][((v_3 & 15u) >> 2u)]));
}

S v_4(uint start_byte_offset) {
  int v_5 = asint(u[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)]);
  matrix<float16_t, 2, 2> v_6 = v_1((4u + start_byte_offset));
  uint v_7 = (64u + start_byte_offset);
  S v_8 = {v_5, v_6, asint(u[(v_7 / 16u)][((v_7 & 15u) >> 2u)])};
  return v_8;
}

typedef S ary_ret[4];
ary_ret v_9(uint start_byte_offset) {
  S a[4] = (S[4])0;
  {
    uint v_10 = 0u;
    v_10 = 0u;
    while(true) {
      uint v_11 = v_10;
      if ((v_11 >= 4u)) {
        break;
      }
      S v_12 = v_4((start_byte_offset + (v_11 * 128u)));
      a[v_11] = v_12;
      {
        v_10 = (v_11 + 1u);
      }
    }
  }
  S v_13[4] = a;
  return v_13;
}

[numthreads(1, 1, 1)]
void f() {
  S v_14[4] = v_9(0u);
  p = v_14;
  S v_15 = v_4(256u);
  p[1u] = v_15;
  p[3u].m = v_1(260u);
  p[1u].m[0u] = tint_bitcast_to_f16(u[0u].z).yx;
}

