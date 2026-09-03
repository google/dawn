
cbuffer cbuffer_a : register(b0) {
  uint4 a[4];
};
RWByteAddressBuffer s : register(u1);
static int counter = int(0);
int i() {
  counter = asint((asuint(counter) + 1u));
  return counter;
}

vector<float16_t, 2> tint_bitcast_to_f16_1(uint src) {
  uint v = src;
  vector<uint16_t, 2> v16 = vector<uint16_t, 2>(((uint2(v, v) >> uint2(0u, 16u)) & (65535u).xx));
  return asfloat16(v16);
}

vector<float16_t, 4> tint_bitcast_to_f16(uint2 src) {
  uint2 v = src;
  vector<uint16_t, 4> v16 = vector<uint16_t, 4>(((v.xxyy >> uint4(0u, 16u, 0u, 16u)) & (65535u).xxxx));
  return asfloat16(v16);
}

matrix<float16_t, 2, 3> v_1(uint start_byte_offset) {
  uint4 v_2 = a[(start_byte_offset / 16u)];
  vector<float16_t, 3> v_3 = tint_bitcast_to_f16(select((((start_byte_offset & 15u) >> 2u) == 2u), v_2.zw, v_2.xy)).xyz;
  uint v_4 = (8u + start_byte_offset);
  uint4 v_5 = a[(v_4 / 16u)];
  return matrix<float16_t, 2, 3>(v_3, tint_bitcast_to_f16(select((((v_4 & 15u) >> 2u) == 2u), v_5.zw, v_5.xy)).xyz);
}

typedef matrix<float16_t, 2, 3> ary_ret[4];
ary_ret v_6(uint start_byte_offset) {
  matrix<float16_t, 2, 3> a_1[4] = (matrix<float16_t, 2, 3>[4])0;
  {
    uint v_7 = 0u;
    v_7 = 0u;
    while(true) {
      uint v_8 = v_7;
      if ((v_8 >= 4u)) {
        break;
      }
      a_1[v_8] = v_1((start_byte_offset + (v_8 * 16u)));
      {
        v_7 = (v_8 + 1u);
      }
    }
  }
  matrix<float16_t, 2, 3> v_9[4] = a_1;
  return v_9;
}

[numthreads(1, 1, 1)]
void f() {
  uint v_10 = (min(uint(i()), 3u) * 16u);
  uint v_11 = (min(uint(i()), 1u) * 8u);
  matrix<float16_t, 2, 3> l_a[4] = v_6(0u);
  matrix<float16_t, 2, 3> l_a_i = v_1(v_10);
  uint v_12 = (v_10 + v_11);
  uint4 v_13 = a[(v_12 / 16u)];
  vector<float16_t, 3> l_a_i_i = tint_bitcast_to_f16(select((((v_12 & 15u) >> 2u) == 2u), v_13.zw, v_13.xy)).xyz;
  uint v_14 = (v_10 + v_11);
  s.Store<float16_t>(0u, (((tint_bitcast_to_f16_1(a[(v_14 / 16u)][((v_14 & 15u) >> 2u)])[select(((v_14 % 4u) == 0u), 0u, 1u)] + l_a[int(0)][int(0)].x) + l_a_i[int(0)].x) + l_a_i_i.x));
}

