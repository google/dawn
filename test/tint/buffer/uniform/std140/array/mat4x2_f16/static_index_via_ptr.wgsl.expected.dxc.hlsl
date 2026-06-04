
cbuffer cbuffer_a : register(b0) {
  uint4 a[4];
};
RWByteAddressBuffer s : register(u1);
vector<float16_t, 2> tint_bitcast_to_f16(uint src) {
  uint v = src;
  uint2 v_1 = uint2(v, v);
  vector<uint16_t, 2> v16 = vector<uint16_t, 2>(((v_1 >> uint2(0u, 16u)) & (65535u).xx));
  return asfloat16(v16);
}

matrix<float16_t, 4, 2> v_2(uint start_byte_offset) {
  vector<float16_t, 2> v_3 = tint_bitcast_to_f16(a[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)]);
  uint v_4 = (4u + start_byte_offset);
  vector<float16_t, 2> v_5 = tint_bitcast_to_f16(a[(v_4 / 16u)][((v_4 & 15u) >> 2u)]);
  uint v_6 = (8u + start_byte_offset);
  vector<float16_t, 2> v_7 = tint_bitcast_to_f16(a[(v_6 / 16u)][((v_6 & 15u) >> 2u)]);
  uint v_8 = (12u + start_byte_offset);
  return matrix<float16_t, 4, 2>(v_3, v_5, v_7, tint_bitcast_to_f16(a[(v_8 / 16u)][((v_8 & 15u) >> 2u)]));
}

typedef matrix<float16_t, 4, 2> ary_ret[4];
ary_ret v_9(uint start_byte_offset) {
  matrix<float16_t, 4, 2> a_1[4] = (matrix<float16_t, 4, 2>[4])0;
  {
    uint v_10 = 0u;
    v_10 = 0u;
    while(true) {
      uint v_11 = v_10;
      if ((v_11 >= 4u)) {
        break;
      }
      a_1[v_11] = v_2((start_byte_offset + (v_11 * 16u)));
      {
        v_10 = (v_11 + 1u);
      }
    }
  }
  matrix<float16_t, 4, 2> v_12[4] = a_1;
  return v_12;
}

[numthreads(1, 1, 1)]
void f() {
  matrix<float16_t, 4, 2> l_a[4] = v_9(0u);
  matrix<float16_t, 4, 2> l_a_i = v_2(32u);
  vector<float16_t, 2> l_a_i_i = tint_bitcast_to_f16(a[2u].y);
  s.Store<float16_t>(0u, (((tint_bitcast_to_f16(a[2u].y).x + l_a[0u][0u].x) + l_a_i[0u].x) + l_a_i_i.x));
}

