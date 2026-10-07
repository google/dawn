
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
    uint idx = 0u;
    while(true) {
      uint v_10 = idx;
      if ((v_10 >= 4u)) {
        break;
      }
      v_1((offset + (v_10 * 16u)), obj[v_10]);
      {
        idx = (idx + 1u);
      }
    }
  }
}

typedef matrix<float16_t, 4, 2> ary_ret[4];
ary_ret v_11(uint start_byte_offset) {
  matrix<float16_t, 4, 2> a[4] = (matrix<float16_t, 4, 2>[4])0;
  {
    uint idx = 0u;
    while(true) {
      uint v_12 = idx;
      if ((v_12 >= 4u)) {
        break;
      }
      a[v_12] = v_2((start_byte_offset + (v_12 * 16u)));
      {
        idx = (idx + 1u);
      }
    }
  }
  matrix<float16_t, 4, 2> v_13[4] = a;
  return v_13;
}

[numthreads(1, 1, 1)]
void f() {
  matrix<float16_t, 4, 2> v_14[4] = v_11(0u);
  v_9(0u, v_14);
  v_1(16u, v_2(32u));
  s.Store<vector<float16_t, 2> >(16u, tint_bitcast_to_f16(u[0u].y).yx);
  s.Store<float16_t>(16u, tint_bitcast_to_f16(u[0u].y).x);
}

