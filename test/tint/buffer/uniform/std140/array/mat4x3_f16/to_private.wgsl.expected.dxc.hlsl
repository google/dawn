
cbuffer cbuffer_u : register(b0) {
  uint4 u[8];
};
RWByteAddressBuffer s : register(u1);
static matrix<float16_t, 4, 3> p[4] = (matrix<float16_t, 4, 3>[4])0;
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

matrix<float16_t, 4, 3> v_2(uint start_byte_offset) {
  uint4 v_3 = u[(start_byte_offset / 16u)];
  vector<float16_t, 3> v_4 = tint_bitcast_to_f16_1(select((((start_byte_offset & 15u) >> 2u) == 2u), v_3.zw, v_3.xy)).xyz;
  uint v_5 = (8u + start_byte_offset);
  uint4 v_6 = u[(v_5 / 16u)];
  vector<float16_t, 3> v_7 = tint_bitcast_to_f16_1(select((((v_5 & 15u) >> 2u) == 2u), v_6.zw, v_6.xy)).xyz;
  uint v_8 = (16u + start_byte_offset);
  uint4 v_9 = u[(v_8 / 16u)];
  vector<float16_t, 3> v_10 = tint_bitcast_to_f16_1(select((((v_8 & 15u) >> 2u) == 2u), v_9.zw, v_9.xy)).xyz;
  uint v_11 = (24u + start_byte_offset);
  uint4 v_12 = u[(v_11 / 16u)];
  return matrix<float16_t, 4, 3>(v_4, v_7, v_10, tint_bitcast_to_f16_1(select((((v_11 & 15u) >> 2u) == 2u), v_12.zw, v_12.xy)).xyz);
}

typedef matrix<float16_t, 4, 3> ary_ret[4];
ary_ret v_13(uint start_byte_offset) {
  matrix<float16_t, 4, 3> a[4] = (matrix<float16_t, 4, 3>[4])0;
  {
    uint v_14 = 0u;
    v_14 = 0u;
    while(true) {
      uint v_15 = v_14;
      if ((v_15 >= 4u)) {
        break;
      }
      a[v_15] = v_2((start_byte_offset + (v_15 * 32u)));
      {
        v_14 = (v_15 + 1u);
      }
    }
  }
  matrix<float16_t, 4, 3> v_16[4] = a;
  return v_16;
}

[numthreads(1, 1, 1)]
void f() {
  matrix<float16_t, 4, 3> v_17[4] = v_13(0u);
  p = v_17;
  p[1u] = v_2(64u);
  p[1u][0u] = tint_bitcast_to_f16_1(u[0u].zw).xyz.zxy;
  p[1u][0u].x = tint_bitcast_to_f16(u[0u].z).x;
  s.Store<float16_t>(0u, p[1u][0u].x);
}

