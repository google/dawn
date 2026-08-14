struct S {
  int before;
  matrix<float16_t, 4, 3> m;
  int after;
};


cbuffer cbuffer_u : register(b0) {
  uint4 u[32];
};
void a(S a_1[4]) {
}

void b(S s) {
}

void c(matrix<float16_t, 4, 3> m) {
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

matrix<float16_t, 4, 3> v_2(uint start_byte_offset) {
  uint4 v_3 = u[(start_byte_offset / 16u)];
  vector<float16_t, 3> v_4 = tint_bitcast_to_f16_1(select((((start_byte_offset & 15u) >> 2u) == 2u), v_3.zw, v_3.xy)).xyz;
  uint v_5 = (8u + start_byte_offset);
  uint4 v_6 = u[(v_5 / 16u)];
  vector<float16_t, 3> v_7 = tint_bitcast_to_f16_1(select((((v_5 & 15u) >> 2u) == 2u), v_6.zw, v_6.xy)).xyz;
  uint v_8 = (16u + start_byte_offset);
  uint4 v_9 = u[(v_8 / 16u)];
  vector<float16_t, 3> v_10 = tint_bitcast_to_f16_1(select((((v_8 & 15u) >> 2u) == 2u), v_9.zw, v_9.xy)).xyz;
  uint v_11 = (24u + start_byte_offset);
  uint4 v_12 = u[(v_11 / 16u)];
  return matrix<float16_t, 4, 3>(v_4, v_7, v_10, tint_bitcast_to_f16_1(select((((v_11 & 15u) >> 2u) == 2u), v_12.zw, v_12.xy)).xyz);
}

S v_13(uint start_byte_offset) {
  int v_14 = asint(u[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)]);
  matrix<float16_t, 4, 3> v_15 = v_2((8u + start_byte_offset));
  uint v_16 = (64u + start_byte_offset);
  S v_17 = {v_14, v_15, asint(u[(v_16 / 16u)][((v_16 & 15u) >> 2u)])};
  return v_17;
}

typedef S ary_ret[4];
ary_ret v_18(uint start_byte_offset) {
  S a_2[4] = (S[4])0;
  {
    uint v_19 = 0u;
    v_19 = 0u;
    while(true) {
      uint v_20 = v_19;
      if ((v_20 >= 4u)) {
        break;
      }
      S v_21 = v_13((start_byte_offset + (v_20 * 128u)));
      a_2[v_20] = v_21;
      {
        v_19 = (v_20 + 1u);
      }
    }
  }
  S v_22[4] = a_2;
  return v_22;
}

[numthreads(1, 1, 1)]
void f() {
  S v_23[4] = v_18(0u);
  a(v_23);
  S v_24 = v_13(256u);
  b(v_24);
  c(v_2(264u));
  d(tint_bitcast_to_f16_1(u[1u].xy).xyz.zxy);
  e(tint_bitcast_to_f16(u[1u].y).x);
}

