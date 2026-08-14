
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

matrix<float16_t, 2, 4> v_2(uint start_byte_offset) {
  uint4 v_3 = u[(start_byte_offset / 16u)];
  vector<float16_t, 4> v_4 = tint_bitcast_to_f16_1(select((((start_byte_offset & 15u) >> 2u) == 2u), v_3.zw, v_3.xy));
  uint v_5 = (8u + start_byte_offset);
  uint4 v_6 = u[(v_5 / 16u)];
  return matrix<float16_t, 2, 4>(v_4, tint_bitcast_to_f16_1(select((((v_5 & 15u) >> 2u) == 2u), v_6.zw, v_6.xy)));
}

typedef matrix<float16_t, 2, 4> ary_ret[4];
ary_ret v_7(uint start_byte_offset) {
  matrix<float16_t, 2, 4> a_2[4] = (matrix<float16_t, 2, 4>[4])0;
  {
    uint v_8 = 0u;
    v_8 = 0u;
    while(true) {
      uint v_9 = v_8;
      if ((v_9 >= 4u)) {
        break;
      }
      a_2[v_9] = v_2((start_byte_offset + (v_9 * 16u)));
      {
        v_8 = (v_9 + 1u);
      }
    }
  }
  matrix<float16_t, 2, 4> v_10[4] = a_2;
  return v_10;
}

[numthreads(1, 1, 1)]
void f() {
  matrix<float16_t, 2, 4> v_11[4] = v_7(0u);
  float16_t v_12 = a(v_11);
  float16_t v_13 = (v_12 + b(v_2(16u)));
  float16_t v_14 = (v_13 + c(tint_bitcast_to_f16_1(u[1u].xy).ywxz));
  s.Store<float16_t>(0u, (v_14 + d(tint_bitcast_to_f16(u[1u].x).y)));
}

