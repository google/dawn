struct S {
  int before;
  matrix<float16_t, 3, 3> m;
  int after;
};


cbuffer cbuffer_u : register(b0) {
  uint4 u[32];
};
void a(S a_1[4]) {
}

void b(S s) {
}

void c(matrix<float16_t, 3, 3> m) {
}

void d(vector<float16_t, 3> v) {
}

void e(float16_t f_1) {
}

vector<float16_t, 2> tint_bitcast_to_f16(uint src) {
  uint v = src;
  uint2 v_1 = uint2(v, v);
  vector<uint16_t, 2> v16 = vector<uint16_t, 2>(((v_1 >> uint2(0u, 16u)) & (65535u).xx));
  return asfloat16(v16);
}

vector<float16_t, 4> tint_bitcast_to_f16_1(uint2 src) {
  uint2 v = src;
  vector<uint16_t, 4> v16 = vector<uint16_t, 4>(((v.xxyy >> uint4(0u, 16u, 0u, 16u)) & (65535u).xxxx));
  return asfloat16(v16);
}

matrix<float16_t, 3, 3> v_2(uint start_byte_offset) {
  uint4 v_3 = u[(start_byte_offset / 16u)];
  vector<float16_t, 3> v_4 = tint_bitcast_to_f16_1(select((((start_byte_offset & 15u) >> 2u) == 2u), v_3.zw, v_3.xy)).xyz;
  uint v_5 = (8u + start_byte_offset);
  uint4 v_6 = u[(v_5 / 16u)];
  vector<float16_t, 3> v_7 = tint_bitcast_to_f16_1(select((((v_5 & 15u) >> 2u) == 2u), v_6.zw, v_6.xy)).xyz;
  uint v_8 = (16u + start_byte_offset);
  uint4 v_9 = u[(v_8 / 16u)];
  return matrix<float16_t, 3, 3>(v_4, v_7, tint_bitcast_to_f16_1(select((((v_8 & 15u) >> 2u) == 2u), v_9.zw, v_9.xy)).xyz);
}

S v_10(uint start_byte_offset) {
  int v_11 = asint(u[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)]);
  matrix<float16_t, 3, 3> v_12 = v_2((8u + start_byte_offset));
  uint v_13 = (64u + start_byte_offset);
  S v_14 = {v_11, v_12, asint(u[(v_13 / 16u)][((v_13 & 15u) >> 2u)])};
  return v_14;
}

typedef S ary_ret[4];
ary_ret v_15(uint start_byte_offset) {
  S a_2[4] = (S[4])0;
  {
    uint v_16 = 0u;
    v_16 = 0u;
    while(true) {
      uint v_17 = v_16;
      if ((v_17 >= 4u)) {
        break;
      }
      S v_18 = v_10((start_byte_offset + (v_17 * 128u)));
      a_2[v_17] = v_18;
      {
        v_16 = (v_17 + 1u);
      }
    }
  }
  S v_19[4] = a_2;
  return v_19;
}

[numthreads(1, 1, 1)]
void f() {
  S v_20[4] = v_15(0u);
  a(v_20);
  S v_21 = v_10(256u);
  b(v_21);
  c(v_2(264u));
  d(tint_bitcast_to_f16_1(u[1u].xy).xyz.zxy);
  e(tint_bitcast_to_f16(u[1u].y).x);
}

