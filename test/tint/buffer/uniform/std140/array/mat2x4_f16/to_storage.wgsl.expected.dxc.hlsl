
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

vector<float16_t, 4> tint_bitcast_to_f16_1(uint2 src) {
  uint2 v = src;
  vector<uint16_t, 4> v16 = vector<uint16_t, 4>(((v.xxyy >> uint4(0u, 16u, 0u, 16u)) & (65535u).xxxx));
  return asfloat16(v16);
}

void v_2(uint offset, matrix<float16_t, 2, 4> obj) {
  s.Store<vector<float16_t, 4> >((offset + 0u), obj[0u]);
  s.Store<vector<float16_t, 4> >((offset + 8u), obj[1u]);
}

matrix<float16_t, 2, 4> v_3(uint start_byte_offset) {
  uint4 v_4 = u[(start_byte_offset / 16u)];
  vector<float16_t, 4> v_5 = tint_bitcast_to_f16_1(select((((start_byte_offset & 15u) >> 2u) == 2u), v_4.zw, v_4.xy));
  uint v_6 = (8u + start_byte_offset);
  uint4 v_7 = u[(v_6 / 16u)];
  return matrix<float16_t, 2, 4>(v_5, tint_bitcast_to_f16_1(select((((v_6 & 15u) >> 2u) == 2u), v_7.zw, v_7.xy)));
}

void v_8(uint offset, matrix<float16_t, 2, 4> obj[4]) {
  {
    uint v_9 = 0u;
    v_9 = 0u;
    while(true) {
      uint v_10 = v_9;
      if ((v_10 >= 4u)) {
        break;
      }
      v_2((offset + (v_10 * 16u)), obj[v_10]);
      {
        v_9 = (v_10 + 1u);
      }
    }
  }
}

typedef matrix<float16_t, 2, 4> ary_ret[4];
ary_ret v_11(uint start_byte_offset) {
  matrix<float16_t, 2, 4> a[4] = (matrix<float16_t, 2, 4>[4])0;
  {
    uint v_12 = 0u;
    v_12 = 0u;
    while(true) {
      uint v_13 = v_12;
      if ((v_13 >= 4u)) {
        break;
      }
      a[v_13] = v_3((start_byte_offset + (v_13 * 16u)));
      {
        v_12 = (v_13 + 1u);
      }
    }
  }
  matrix<float16_t, 2, 4> v_14[4] = a;
  return v_14;
}

[numthreads(1, 1, 1)]
void f() {
  matrix<float16_t, 2, 4> v_15[4] = v_11(0u);
  v_8(0u, v_15);
  v_2(16u, v_3(32u));
  s.Store<vector<float16_t, 4> >(16u, tint_bitcast_to_f16_1(u[0u].zw).ywxz);
  s.Store<float16_t>(16u, tint_bitcast_to_f16(u[0u].z).x);
}

