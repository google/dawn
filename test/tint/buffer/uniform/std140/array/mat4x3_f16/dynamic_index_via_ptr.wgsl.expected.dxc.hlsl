
cbuffer cbuffer_a : register(b0) {
  uint4 a[8];
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

matrix<float16_t, 4, 3> v_1(uint start_byte_offset) {
  uint4 v_2 = a[(start_byte_offset / 16u)];
  vector<float16_t, 3> v_3 = tint_bitcast_to_f16(select((((start_byte_offset & 15u) >> 2u) == 2u), v_2.zw, v_2.xy)).xyz;
  uint v_4 = (8u + start_byte_offset);
  uint4 v_5 = a[(v_4 / 16u)];
  vector<float16_t, 3> v_6 = tint_bitcast_to_f16(select((((v_4 & 15u) >> 2u) == 2u), v_5.zw, v_5.xy)).xyz;
  uint v_7 = (16u + start_byte_offset);
  uint4 v_8 = a[(v_7 / 16u)];
  vector<float16_t, 3> v_9 = tint_bitcast_to_f16(select((((v_7 & 15u) >> 2u) == 2u), v_8.zw, v_8.xy)).xyz;
  uint v_10 = (24u + start_byte_offset);
  uint4 v_11 = a[(v_10 / 16u)];
  return matrix<float16_t, 4, 3>(v_3, v_6, v_9, tint_bitcast_to_f16(select((((v_10 & 15u) >> 2u) == 2u), v_11.zw, v_11.xy)).xyz);
}

typedef matrix<float16_t, 4, 3> ary_ret[4];
ary_ret v_12(uint start_byte_offset) {
  matrix<float16_t, 4, 3> a_1[4] = (matrix<float16_t, 4, 3>[4])0;
  {
    uint v_13 = 0u;
    v_13 = 0u;
    while(true) {
      uint v_14 = v_13;
      if ((v_14 >= 4u)) {
        break;
      }
      a_1[v_14] = v_1((start_byte_offset + (v_14 * 32u)));
      {
        v_13 = (v_14 + 1u);
      }
    }
  }
  matrix<float16_t, 4, 3> v_15[4] = a_1;
  return v_15;
}

[numthreads(1, 1, 1)]
void f() {
  uint v_16 = (min(uint(i()), 3u) * 32u);
  uint v_17 = (min(uint(i()), 3u) * 8u);
  matrix<float16_t, 4, 3> l_a[4] = v_12(0u);
  matrix<float16_t, 4, 3> l_a_i = v_1(v_16);
  uint v_18 = (v_16 + v_17);
  uint4 v_19 = a[(v_18 / 16u)];
  vector<float16_t, 3> l_a_i_i = tint_bitcast_to_f16(select((((v_18 & 15u) >> 2u) == 2u), v_19.zw, v_19.xy)).xyz;
  uint v_20 = (v_16 + v_17);
  s.Store<float16_t>(0u, (((tint_bitcast_to_f16_1(a[(v_20 / 16u)][((v_20 & 15u) >> 2u)])[select(((v_20 % 4u) == 0u), 0u, 1u)] + l_a[int(0)][int(0)].x) + l_a_i[int(0)].x) + l_a_i_i.x));
}

