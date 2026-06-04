
cbuffer cbuffer_u : register(b0) {
  uint4 u[8];
};
RWByteAddressBuffer s : register(u1);
float16_t a(matrix<float16_t, 4, 3> a_1[4]) {
  return a_1[0u][0u].x;
}

float16_t b(matrix<float16_t, 4, 3> m) {
  return m[0u].x;
}

float16_t c(vector<float16_t, 3> v) {
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

matrix<float16_t, 4, 3> v_1(uint start_byte_offset) {
  uint4 v_2 = u[(start_byte_offset / 16u)];
  vector<float16_t, 3> v_3 = tint_bitcast_to_f16(select((((start_byte_offset & 15u) >> 2u) == 2u), v_2.zw, v_2.xy)).xyz;
  uint v_4 = (8u + start_byte_offset);
  uint4 v_5 = u[(v_4 / 16u)];
  vector<float16_t, 3> v_6 = tint_bitcast_to_f16(select((((v_4 & 15u) >> 2u) == 2u), v_5.zw, v_5.xy)).xyz;
  uint v_7 = (16u + start_byte_offset);
  uint4 v_8 = u[(v_7 / 16u)];
  vector<float16_t, 3> v_9 = tint_bitcast_to_f16(select((((v_7 & 15u) >> 2u) == 2u), v_8.zw, v_8.xy)).xyz;
  uint v_10 = (24u + start_byte_offset);
  uint4 v_11 = u[(v_10 / 16u)];
  return matrix<float16_t, 4, 3>(v_3, v_6, v_9, tint_bitcast_to_f16(select((((v_10 & 15u) >> 2u) == 2u), v_11.zw, v_11.xy)).xyz);
}

typedef matrix<float16_t, 4, 3> ary_ret[4];
ary_ret v_12(uint start_byte_offset) {
  matrix<float16_t, 4, 3> a_2[4] = (matrix<float16_t, 4, 3>[4])0;
  {
    uint v_13 = 0u;
    v_13 = 0u;
    while(true) {
      uint v_14 = v_13;
      if ((v_14 >= 4u)) {
        break;
      }
      a_2[v_14] = v_1((start_byte_offset + (v_14 * 32u)));
      {
        v_13 = (v_14 + 1u);
      }
    }
  }
  matrix<float16_t, 4, 3> v_15[4] = a_2;
  return v_15;
}

[numthreads(1, 1, 1)]
void f() {
  matrix<float16_t, 4, 3> v_16[4] = v_12(0u);
  float16_t v_17 = a(v_16);
  float16_t v_18 = (v_17 + b(v_1(32u)));
  float16_t v_19 = (v_18 + c(tint_bitcast_to_f16(u[2u].xy).xyz.zxy));
  s.Store<float16_t>(0u, (v_19 + d(tint_bitcast_to_f16(u[2u].xy).xyz.zxy.x)));
}

