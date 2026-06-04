
cbuffer cbuffer_u : register(b0) {
  uint4 u[4];
};
RWByteAddressBuffer s : register(u1);
vector<float16_t, 2> tint_bitcast_to_f16(uint src) {
  uint v = src;
  uint2 v_1 = uint2(v, v);
  vector<uint16_t, 2> v16 = vector<uint16_t, 2>(((v_1 >> uint2(0u, 16u)) & (65535u).xx));
  return asfloat16(v16);
}

void v_2(uint offset, matrix<float16_t, 4, 2> obj) {
  s.Store<vector<float16_t, 2> >((offset + 0u), obj[0u]);
  s.Store<vector<float16_t, 2> >((offset + 4u), obj[1u]);
  s.Store<vector<float16_t, 2> >((offset + 8u), obj[2u]);
  s.Store<vector<float16_t, 2> >((offset + 12u), obj[3u]);
}

matrix<float16_t, 4, 2> v_3(uint start_byte_offset) {
  vector<float16_t, 2> v_4 = tint_bitcast_to_f16(u[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)]);
  uint v_5 = (4u + start_byte_offset);
  vector<float16_t, 2> v_6 = tint_bitcast_to_f16(u[(v_5 / 16u)][((v_5 & 15u) >> 2u)]);
  uint v_7 = (8u + start_byte_offset);
  vector<float16_t, 2> v_8 = tint_bitcast_to_f16(u[(v_7 / 16u)][((v_7 & 15u) >> 2u)]);
  uint v_9 = (12u + start_byte_offset);
  return matrix<float16_t, 4, 2>(v_4, v_6, v_8, tint_bitcast_to_f16(u[(v_9 / 16u)][((v_9 & 15u) >> 2u)]));
}

void v_10(uint offset, matrix<float16_t, 4, 2> obj[4]) {
  {
    uint v_11 = 0u;
    v_11 = 0u;
    while(true) {
      uint v_12 = v_11;
      if ((v_12 >= 4u)) {
        break;
      }
      v_2((offset + (v_12 * 16u)), obj[v_12]);
      {
        v_11 = (v_12 + 1u);
      }
    }
  }
}

typedef matrix<float16_t, 4, 2> ary_ret[4];
ary_ret v_13(uint start_byte_offset) {
  matrix<float16_t, 4, 2> a[4] = (matrix<float16_t, 4, 2>[4])0;
  {
    uint v_14 = 0u;
    v_14 = 0u;
    while(true) {
      uint v_15 = v_14;
      if ((v_15 >= 4u)) {
        break;
      }
      a[v_15] = v_3((start_byte_offset + (v_15 * 16u)));
      {
        v_14 = (v_15 + 1u);
      }
    }
  }
  matrix<float16_t, 4, 2> v_16[4] = a;
  return v_16;
}

[numthreads(1, 1, 1)]
void f() {
  matrix<float16_t, 4, 2> v_17[4] = v_13(0u);
  v_10(0u, v_17);
  v_2(16u, v_3(32u));
  s.Store<vector<float16_t, 2> >(16u, tint_bitcast_to_f16(u[0u].y).yx);
  s.Store<float16_t>(16u, tint_bitcast_to_f16(u[0u].y).x);
}

