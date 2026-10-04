
cbuffer cbuffer_u : register(b0) {
  uint4 u[8];
};
RWByteAddressBuffer s : register(u1);
float16_t a(matrix<float16_t, 4, 4> a_1[4]) {
  return a_1[int(0)][int(0)].x;
}

float16_t b(matrix<float16_t, 4, 4> m) {
  return m[int(0)].x;
}

float16_t c(vector<float16_t, 4> v) {
  return v.x;
}

float16_t d(float16_t f_1) {
  return f_1;
}

vector<float16_t, 2> tint_bitcast_to_f16(uint src) {
  uint v = src;
  vector<uint16_t, 2> v16 = vector<uint16_t, 2>(((uint2(v, v) >> uint2(0u, 16u)) & (65535u).xx));
  return asfloat16(v16);
}

vector<float16_t, 4> tint_bitcast_to_f16_1(uint2 src) {
  uint2 v = src;
  vector<uint16_t, 4> v16 = vector<uint16_t, 4>(((v.xxyy >> uint4(0u, 16u, 0u, 16u)) & (65535u).xxxx));
  return asfloat16(v16);
}

matrix<float16_t, 4, 4> v_1(uint start_byte_offset) {
  uint4 v_2 = u[(start_byte_offset / 16u)];
  vector<float16_t, 4> v_3 = tint_bitcast_to_f16_1(select((((start_byte_offset & 15u) >> 2u) == 2u), v_2.zw, v_2.xy));
  uint v_4 = (8u + start_byte_offset);
  uint4 v_5 = u[(v_4 / 16u)];
  vector<float16_t, 4> v_6 = tint_bitcast_to_f16_1(select((((v_4 & 15u) >> 2u) == 2u), v_5.zw, v_5.xy));
  uint v_7 = (16u + start_byte_offset);
  uint4 v_8 = u[(v_7 / 16u)];
  vector<float16_t, 4> v_9 = tint_bitcast_to_f16_1(select((((v_7 & 15u) >> 2u) == 2u), v_8.zw, v_8.xy));
  uint v_10 = (24u + start_byte_offset);
  uint4 v_11 = u[(v_10 / 16u)];
  return matrix<float16_t, 4, 4>(v_3, v_6, v_9, tint_bitcast_to_f16_1(select((((v_10 & 15u) >> 2u) == 2u), v_11.zw, v_11.xy)));
}

typedef matrix<float16_t, 4, 4> ary_ret[4];
ary_ret v_12(uint start_byte_offset) {
  matrix<float16_t, 4, 4> a_2[4] = (matrix<float16_t, 4, 4>[4])0;
  {
    uint idx = 0u;
    while(true) {
      uint v_13 = idx;
      if ((v_13 >= 4u)) {
        break;
      }
      a_2[v_13] = v_1((start_byte_offset + (v_13 * 32u)));
      {
        idx = (idx + 1u);
      }
    }
  }
  matrix<float16_t, 4, 4> v_14[4] = a_2;
  return v_14;
}

[numthreads(1, 1, 1)]
void f() {
  matrix<float16_t, 4, 4> v_15[4] = v_12(0u);
  float16_t v_16 = a(v_15);
  float16_t v_17 = (v_16 + b(v_1(32u)));
  float16_t v_18 = (v_17 + c(tint_bitcast_to_f16_1(u[2u].xy).ywxz));
  s.Store<float16_t>(0u, (v_18 + d(tint_bitcast_to_f16(u[2u].x).y)));
}

