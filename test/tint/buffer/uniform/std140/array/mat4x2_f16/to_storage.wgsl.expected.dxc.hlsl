
cbuffer cbuffer_u : register(b0) {
  uint4 u[4];
};
RWByteAddressBuffer s : register(u1);
vector<float16_t, 2> tint_bitcast_to_f16(uint src) {
  uint v = src;
  vector<uint16_t, 2> v16 = vector<uint16_t, 2>(((uint2(v, v) >> uint2(0u, 16u)) & (65535u).xx));
  return asfloat16(v16);
}

void v_1(uint offset, matrix<float16_t, 4, 2> obj) {
  s.Store<vector<float16_t, 2> >((offset + 0u), obj[0u]);
  s.Store<vector<float16_t, 2> >((offset + 4u), obj[1u]);
  s.Store<vector<float16_t, 2> >((offset + 8u), obj[2u]);
  s.Store<vector<float16_t, 2> >((offset + 12u), obj[3u]);
}

matrix<float16_t, 4, 2> v_2(uint start_byte_offset) {
  vector<float16_t, 2> v_3 = tint_bitcast_to_f16(u[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)]);
  uint v_4 = (4u + start_byte_offset);
  vector<float16_t, 2> v_5 = tint_bitcast_to_f16(u[(v_4 / 16u)][((v_4 & 15u) >> 2u)]);
  uint v_6 = (8u + start_byte_offset);
  vector<float16_t, 2> v_7 = tint_bitcast_to_f16(u[(v_6 / 16u)][((v_6 & 15u) >> 2u)]);
  uint v_8 = (12u + start_byte_offset);
  return matrix<float16_t, 4, 2>(v_3, v_5, v_7, tint_bitcast_to_f16(u[(v_8 / 16u)][((v_8 & 15u) >> 2u)]));
}

void v_9(uint offset, matrix<float16_t, 4, 2> obj[4]) {
  {
    uint v_10 = 0u;
    v_10 = 0u;
    while(true) {
      uint v_11 = v_10;
      if ((v_11 >= 4u)) {
        break;
      }
      v_1((offset + (v_11 * 16u)), obj[v_11]);
      {
        v_10 = (v_11 + 1u);
      }
    }
  }
}

typedef matrix<float16_t, 4, 2> ary_ret[4];
ary_ret v_12(uint start_byte_offset) {
  matrix<float16_t, 4, 2> a[4] = (matrix<float16_t, 4, 2>[4])0;
  {
    uint v_13 = 0u;
    v_13 = 0u;
    while(true) {
      uint v_14 = v_13;
      if ((v_14 >= 4u)) {
        break;
      }
      a[v_14] = v_2((start_byte_offset + (v_14 * 16u)));
      {
        v_13 = (v_14 + 1u);
      }
    }
  }
  matrix<float16_t, 4, 2> v_15[4] = a;
  return v_15;
}

[numthreads(1, 1, 1)]
void f() {
  matrix<float16_t, 4, 2> v_16[4] = v_12(0u);
  v_9(0u, v_16);
  v_1(16u, v_2(32u));
  s.Store<vector<float16_t, 2> >(16u, tint_bitcast_to_f16(u[0u].y).yx);
  s.Store<float16_t>(16u, tint_bitcast_to_f16(u[0u].y).x);
}

