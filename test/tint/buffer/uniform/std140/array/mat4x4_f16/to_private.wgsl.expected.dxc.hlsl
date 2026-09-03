
cbuffer cbuffer_u : register(b0) {
  uint4 u[8];
};
RWByteAddressBuffer s : register(u1);
static matrix<float16_t, 4, 4> p[4] = (matrix<float16_t, 4, 4>[4])0;
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

matrix<float16_t, 4, 4> v_1(uint start_byte_offset) {
  uint4 v_2 = u[(start_byte_offset / 16u)];
  vector<float16_t, 4> v_3 = tint_bitcast_to_f16_1(select((((start_byte_offset & 15u) >> 2u) == 2u), v_2.zw, v_2.xy));
  uint v_4 = (8u + start_byte_offset);
  uint4 v_5 = u[(v_4 / 16u)];
  vector<float16_t, 4> v_6 = tint_bitcast_to_f16_1(select((((v_4 & 15u) >> 2u) == 2u), v_5.zw, v_5.xy));
  uint v_7 = (16u + start_byte_offset);
  uint4 v_8 = u[(v_7 / 16u)];
  vector<float16_t, 4> v_9 = tint_bitcast_to_f16_1(select((((v_7 & 15u) >> 2u) == 2u), v_8.zw, v_8.xy));
  uint v_10 = (24u + start_byte_offset);
  uint4 v_11 = u[(v_10 / 16u)];
  return matrix<float16_t, 4, 4>(v_3, v_6, v_9, tint_bitcast_to_f16_1(select((((v_10 & 15u) >> 2u) == 2u), v_11.zw, v_11.xy)));
}

typedef matrix<float16_t, 4, 4> ary_ret[4];
ary_ret v_12(uint start_byte_offset) {
  matrix<float16_t, 4, 4> a[4] = (matrix<float16_t, 4, 4>[4])0;
  {
    uint v_13 = 0u;
    v_13 = 0u;
    while(true) {
      uint v_14 = v_13;
      if ((v_14 >= 4u)) {
        break;
      }
      a[v_14] = v_1((start_byte_offset + (v_14 * 32u)));
      {
        v_13 = (v_14 + 1u);
      }
    }
  }
  matrix<float16_t, 4, 4> v_15[4] = a;
  return v_15;
}

[numthreads(1, 1, 1)]
void f() {
  matrix<float16_t, 4, 4> v_16[4] = v_12(0u);
  p = v_16;
  p[int(1)] = v_1(64u);
  p[int(1)][int(0)] = tint_bitcast_to_f16_1(u[0u].zw).ywxz;
  p[int(1)][int(0)].x = tint_bitcast_to_f16(u[0u].z).x;
  s.Store<float16_t>(0u, p[int(1)][int(0)].x);
}

