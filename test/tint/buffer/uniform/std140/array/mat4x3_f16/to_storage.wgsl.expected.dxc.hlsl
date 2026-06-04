
cbuffer cbuffer_u : register(b0) {
  uint4 u[8];
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

void v_2(uint offset, matrix<float16_t, 4, 3> obj) {
  s.Store<vector<float16_t, 3> >((offset + 0u), obj[0u]);
  s.Store<vector<float16_t, 3> >((offset + 8u), obj[1u]);
  s.Store<vector<float16_t, 3> >((offset + 16u), obj[2u]);
  s.Store<vector<float16_t, 3> >((offset + 24u), obj[3u]);
}

matrix<float16_t, 4, 3> v_3(uint start_byte_offset) {
  uint4 v_4 = u[(start_byte_offset / 16u)];
  vector<float16_t, 3> v_5 = tint_bitcast_to_f16_1(select((((start_byte_offset & 15u) >> 2u) == 2u), v_4.zw, v_4.xy)).xyz;
  uint v_6 = (8u + start_byte_offset);
  uint4 v_7 = u[(v_6 / 16u)];
  vector<float16_t, 3> v_8 = tint_bitcast_to_f16_1(select((((v_6 & 15u) >> 2u) == 2u), v_7.zw, v_7.xy)).xyz;
  uint v_9 = (16u + start_byte_offset);
  uint4 v_10 = u[(v_9 / 16u)];
  vector<float16_t, 3> v_11 = tint_bitcast_to_f16_1(select((((v_9 & 15u) >> 2u) == 2u), v_10.zw, v_10.xy)).xyz;
  uint v_12 = (24u + start_byte_offset);
  uint4 v_13 = u[(v_12 / 16u)];
  return matrix<float16_t, 4, 3>(v_5, v_8, v_11, tint_bitcast_to_f16_1(select((((v_12 & 15u) >> 2u) == 2u), v_13.zw, v_13.xy)).xyz);
}

void v_14(uint offset, matrix<float16_t, 4, 3> obj[4]) {
  {
    uint v_15 = 0u;
    v_15 = 0u;
    while(true) {
      uint v_16 = v_15;
      if ((v_16 >= 4u)) {
        break;
      }
      v_2((offset + (v_16 * 32u)), obj[v_16]);
      {
        v_15 = (v_16 + 1u);
      }
    }
  }
}

typedef matrix<float16_t, 4, 3> ary_ret[4];
ary_ret v_17(uint start_byte_offset) {
  matrix<float16_t, 4, 3> a[4] = (matrix<float16_t, 4, 3>[4])0;
  {
    uint v_18 = 0u;
    v_18 = 0u;
    while(true) {
      uint v_19 = v_18;
      if ((v_19 >= 4u)) {
        break;
      }
      a[v_19] = v_3((start_byte_offset + (v_19 * 32u)));
      {
        v_18 = (v_19 + 1u);
      }
    }
  }
  matrix<float16_t, 4, 3> v_20[4] = a;
  return v_20;
}

[numthreads(1, 1, 1)]
void f() {
  matrix<float16_t, 4, 3> v_21[4] = v_17(0u);
  v_14(0u, v_21);
  v_2(32u, v_3(64u));
  s.Store<vector<float16_t, 3> >(32u, tint_bitcast_to_f16_1(u[0u].zw).xyz.zxy);
  s.Store<float16_t>(32u, tint_bitcast_to_f16(u[0u].z).x);
}

