
cbuffer cbuffer_u : register(b0) {
  uint4 u[4];
};
RWByteAddressBuffer s : register(u1);
float16_t a(matrix<float16_t, 2, 4> a_1[4]) {
  return a_1[0u][0u].x;
}

float16_t b(matrix<float16_t, 2, 4> m) {
  return m[0u].x;
}

float16_t c(vector<float16_t, 4> v) {
  return v.x;
}

float16_t d(float16_t f_1) {
  return f_1;
}

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

typedef matrix<float16_t, 2, 4> ary_ret[4];
ary_ret v_6(uint start_byte_offset) {
  matrix<float16_t, 2, 4> a_2[4] = (matrix<float16_t, 2, 4>[4])0;
  {
    uint v_7 = 0u;
    v_7 = 0u;
    while(true) {
      uint v_8 = v_7;
      if ((v_8 >= 4u)) {
        break;
      }
      a_2[v_8] = v_1((start_byte_offset + (v_8 * 16u)));
      {
        v_7 = (v_8 + 1u);
      }
    }
  }
  matrix<float16_t, 2, 4> v_9[4] = a_2;
  return v_9;
}

[numthreads(1, 1, 1)]
void f() {
  matrix<float16_t, 2, 4> v_10[4] = v_6(0u);
  float16_t v_11 = a(v_10);
  float16_t v_12 = (v_11 + b(v_1(16u)));
  float16_t v_13 = (v_12 + c(tint_bitcast_to_f16(u[1u].xy).ywxz));
  s.Store<float16_t>(0u, (v_13 + d(tint_bitcast_to_f16(u[1u].xy).ywxz.x)));
}

