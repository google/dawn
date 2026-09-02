
cbuffer cbuffer_a : register(b0) {
  uint4 a[4];
};
RWByteAddressBuffer s : register(u1);
vector<float16_t, 2> tint_bitcast_to_f16(uint src) {
  uint v = src;
  vector<uint16_t, 2> v16 = vector<uint16_t, 2>(((uint2(v, v) >> uint2(0u, 16u)) & (65535u).xx));
  return asfloat16(v16);
}

matrix<float16_t, 4, 2> v_1(uint start_byte_offset) {
  vector<float16_t, 2> v_2 = tint_bitcast_to_f16(a[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)]);
  uint v_3 = (4u + start_byte_offset);
  vector<float16_t, 2> v_4 = tint_bitcast_to_f16(a[(v_3 / 16u)][((v_3 & 15u) >> 2u)]);
  uint v_5 = (8u + start_byte_offset);
  vector<float16_t, 2> v_6 = tint_bitcast_to_f16(a[(v_5 / 16u)][((v_5 & 15u) >> 2u)]);
  uint v_7 = (12u + start_byte_offset);
  return matrix<float16_t, 4, 2>(v_2, v_4, v_6, tint_bitcast_to_f16(a[(v_7 / 16u)][((v_7 & 15u) >> 2u)]));
}

typedef matrix<float16_t, 4, 2> ary_ret[4];
ary_ret v_8(uint start_byte_offset) {
  matrix<float16_t, 4, 2> a_1[4] = (matrix<float16_t, 4, 2>[4])0;
  {
    uint v_9 = 0u;
    v_9 = 0u;
    while(true) {
      uint v_10 = v_9;
      if ((v_10 >= 4u)) {
        break;
      }
      a_1[v_10] = v_1((start_byte_offset + (v_10 * 16u)));
      {
        v_9 = (v_10 + 1u);
      }
    }
  }
  matrix<float16_t, 4, 2> v_11[4] = a_1;
  return v_11;
}

[numthreads(1, 1, 1)]
void f() {
  matrix<float16_t, 4, 2> l_a[4] = v_8(0u);
  matrix<float16_t, 4, 2> l_a_i = v_1(32u);
  vector<float16_t, 2> l_a_i_i = tint_bitcast_to_f16(a[2u].y);
  s.Store<float16_t>(0u, (((tint_bitcast_to_f16(a[2u].y).x + l_a[0u][0u].x) + l_a_i[0u].x) + l_a_i_i.x));
}

