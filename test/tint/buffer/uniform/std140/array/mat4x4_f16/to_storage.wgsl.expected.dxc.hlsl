
cbuffer cbuffer_u : register(b0) {
  uint4 u[8];
};
RWByteAddressBuffer s : register(u1);
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

void v_1(uint offset, matrix<float16_t, 4, 4> obj) {
  s.Store<vector<float16_t, 4> >((offset + 0u), obj[0u]);
  s.Store<vector<float16_t, 4> >((offset + 8u), obj[1u]);
  s.Store<vector<float16_t, 4> >((offset + 16u), obj[2u]);
  s.Store<vector<float16_t, 4> >((offset + 24u), obj[3u]);
}

matrix<float16_t, 4, 4> v_2(uint start_byte_offset) {
  uint4 v_3 = u[(start_byte_offset / 16u)];
  vector<float16_t, 4> v_4 = tint_bitcast_to_f16_1(select((((start_byte_offset & 15u) >> 2u) == 2u), v_3.zw, v_3.xy));
  uint v_5 = (8u + start_byte_offset);
  uint4 v_6 = u[(v_5 / 16u)];
  vector<float16_t, 4> v_7 = tint_bitcast_to_f16_1(select((((v_5 & 15u) >> 2u) == 2u), v_6.zw, v_6.xy));
  uint v_8 = (16u + start_byte_offset);
  uint4 v_9 = u[(v_8 / 16u)];
  vector<float16_t, 4> v_10 = tint_bitcast_to_f16_1(select((((v_8 & 15u) >> 2u) == 2u), v_9.zw, v_9.xy));
  uint v_11 = (24u + start_byte_offset);
  uint4 v_12 = u[(v_11 / 16u)];
  return matrix<float16_t, 4, 4>(v_4, v_7, v_10, tint_bitcast_to_f16_1(select((((v_11 & 15u) >> 2u) == 2u), v_12.zw, v_12.xy)));
}

void v_13(uint offset, matrix<float16_t, 4, 4> obj[4]) {
  {
    uint v_14 = 0u;
    v_14 = 0u;
    while(true) {
      uint v_15 = v_14;
      if ((v_15 >= 4u)) {
        break;
      }
      v_1((offset + (v_15 * 32u)), obj[v_15]);
      {
        v_14 = (v_15 + 1u);
      }
    }
  }
}

typedef matrix<float16_t, 4, 4> ary_ret[4];
ary_ret v_16(uint start_byte_offset) {
  matrix<float16_t, 4, 4> a[4] = (matrix<float16_t, 4, 4>[4])0;
  {
    uint v_17 = 0u;
    v_17 = 0u;
    while(true) {
      uint v_18 = v_17;
      if ((v_18 >= 4u)) {
        break;
      }
      a[v_18] = v_2((start_byte_offset + (v_18 * 32u)));
      {
        v_17 = (v_18 + 1u);
      }
    }
  }
  matrix<float16_t, 4, 4> v_19[4] = a;
  return v_19;
}

[numthreads(1, 1, 1)]
void f() {
  matrix<float16_t, 4, 4> v_20[4] = v_16(0u);
  v_13(0u, v_20);
  v_1(32u, v_2(64u));
  s.Store<vector<float16_t, 4> >(32u, tint_bitcast_to_f16_1(u[0u].zw).ywxz);
  s.Store<float16_t>(32u, tint_bitcast_to_f16(u[0u].z).x);
}

