struct S {
  int before;
  matrix<float16_t, 3, 4> m;
  int after;
};


cbuffer cbuffer_u : register(b0) {
  uint4 u[32];
};
static S p[4] = (S[4])0;
vector<float16_t, 4> tint_bitcast_to_f16(uint2 src) {
  uint2 v = src;
  vector<uint16_t, 4> v16 = vector<uint16_t, 4>(((v.xxyy >> uint4(0u, 16u, 0u, 16u)) & (65535u).xxxx));
  return asfloat16(v16);
}

matrix<float16_t, 3, 4> v_1(uint start_byte_offset) {
  uint4 v_2 = u[(start_byte_offset / 16u)];
  vector<float16_t, 4> v_3 = tint_bitcast_to_f16(select((((start_byte_offset & 15u) >> 2u) == 2u), v_2.zw, v_2.xy));
  uint v_4 = (8u + start_byte_offset);
  uint4 v_5 = u[(v_4 / 16u)];
  vector<float16_t, 4> v_6 = tint_bitcast_to_f16(select((((v_4 & 15u) >> 2u) == 2u), v_5.zw, v_5.xy));
  uint v_7 = (16u + start_byte_offset);
  uint4 v_8 = u[(v_7 / 16u)];
  return matrix<float16_t, 3, 4>(v_3, v_6, tint_bitcast_to_f16(select((((v_7 & 15u) >> 2u) == 2u), v_8.zw, v_8.xy)));
}

S v_9(uint start_byte_offset) {
  int v_10 = asint(u[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)]);
  matrix<float16_t, 3, 4> v_11 = v_1((8u + start_byte_offset));
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

[numthreads(1, 1, 1)]
void f() {
  S v_19[4] = v_14(0u);
  p = v_19;
  S v_20 = v_9(256u);
  p[1u] = v_20;
  p[3u].m = v_1(264u);
  p[1u].m[0u] = tint_bitcast_to_f16(u[1u].xy).ywxz;
}

