
cbuffer cbuffer_u : register(b0) {
  uint4 u[4];
};
RWByteAddressBuffer s : register(u1);
static matrix<float16_t, 2, 4> p[4] = (matrix<float16_t, 2, 4>[4])0;
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

matrix<float16_t, 2, 4> v_1(uint start_byte_offset) {
  uint4 v_2 = u[(start_byte_offset / 16u)];
  vector<float16_t, 4> v_3 = tint_bitcast_to_f16_1(select((((start_byte_offset & 15u) >> 2u) == 2u), v_2.zw, v_2.xy));
  uint v_4 = (8u + start_byte_offset);
  uint4 v_5 = u[(v_4 / 16u)];
  return matrix<float16_t, 2, 4>(v_3, tint_bitcast_to_f16_1(select((((v_4 & 15u) >> 2u) == 2u), v_5.zw, v_5.xy)));
}

typedef matrix<float16_t, 2, 4> ary_ret[4];
ary_ret v_6(uint start_byte_offset) {
  matrix<float16_t, 2, 4> a[4] = (matrix<float16_t, 2, 4>[4])0;
  {
    uint v_7 = 0u;
    v_7 = 0u;
    while(true) {
      uint v_8 = v_7;
      if ((v_8 >= 4u)) {
        break;
      }
      a[v_8] = v_1((start_byte_offset + (v_8 * 16u)));
      {
        v_7 = (v_8 + 1u);
      }
    }
  }
  matrix<float16_t, 2, 4> v_9[4] = a;
  return v_9;
}

[numthreads(1, 1, 1)]
void f() {
  matrix<float16_t, 2, 4> v_10[4] = v_6(0u);
  p = v_10;
  p[1u] = v_1(32u);
  p[1u][0u] = tint_bitcast_to_f16_1(u[0u].zw).ywxz;
  p[1u][0u].x = tint_bitcast_to_f16(u[0u].z).x;
  s.Store<float16_t>(0u, p[1u][0u].x);
}

