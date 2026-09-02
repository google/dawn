
cbuffer cbuffer_u : register(b0) {
  uint4 u[4];
};
RWByteAddressBuffer s : register(u1);
static matrix<float16_t, 4, 2> p[4] = (matrix<float16_t, 4, 2>[4])0;
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

typedef matrix<float16_t, 4, 2> ary_ret[4];
ary_ret v_8(uint start_byte_offset) {
  matrix<float16_t, 4, 2> a[4] = (matrix<float16_t, 4, 2>[4])0;
  {
    uint v_9 = 0u;
    v_9 = 0u;
    while(true) {
      uint v_10 = v_9;
      if ((v_10 >= 4u)) {
        break;
      }
      a[v_10] = v_1((start_byte_offset + (v_10 * 16u)));
      {
        v_9 = (v_10 + 1u);
      }
    }
  }
  matrix<float16_t, 4, 2> v_11[4] = a;
  return v_11;
}

[numthreads(1, 1, 1)]
void f() {
  matrix<float16_t, 4, 2> v_12[4] = v_8(0u);
  p = v_12;
  p[1u] = v_1(32u);
  p[1u][0u] = tint_bitcast_to_f16(u[0u].y).yx;
  p[1u][0u].x = tint_bitcast_to_f16(u[0u].y).x;
  s.Store<float16_t>(0u, p[1u][0u].x);
}

