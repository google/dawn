
cbuffer cbuffer_a : register(b0) {
  uint4 a[4];
};
RWByteAddressBuffer s : register(u1);
static int counter = int(0);
int i() {
  counter = asint((asuint(counter) + asuint(int(1))));
  return counter;
}

vector<float16_t, 2> tint_bitcast_to_f16_1(uint src) {
  uint v = src;
  uint2 v_1 = uint2(v, v);
  vector<uint16_t, 2> v16 = vector<uint16_t, 2>(((v_1 >> uint2(0u, 16u)) & (65535u).xx));
  return asfloat16(v16);
}

vector<float16_t, 4> tint_bitcast_to_f16(uint2 src) {
  uint2 v = src;
  vector<uint16_t, 4> v16 = vector<uint16_t, 4>(((v.xxyy >> uint4(0u, 16u, 0u, 16u)) & (65535u).xxxx));
  return asfloat16(v16);
}

matrix<float16_t, 2, 3> v_2(uint start_byte_offset) {
  uint4 v_3 = a[(start_byte_offset / 16u)];
  vector<float16_t, 3> v_4 = tint_bitcast_to_f16(select((((start_byte_offset & 15u) >> 2u) == 2u), v_3.zw, v_3.xy)).xyz;
  uint v_5 = (8u + start_byte_offset);
  uint4 v_6 = a[(v_5 / 16u)];
  return matrix<float16_t, 2, 3>(v_4, tint_bitcast_to_f16(select((((v_5 & 15u) >> 2u) == 2u), v_6.zw, v_6.xy)).xyz);
}

typedef matrix<float16_t, 2, 3> ary_ret[4];
ary_ret v_7(uint start_byte_offset) {
  matrix<float16_t, 2, 3> a_1[4] = (matrix<float16_t, 2, 3>[4])0;
  {
    uint v_8 = 0u;
    v_8 = 0u;
    while(true) {
      uint v_9 = v_8;
      if ((v_9 >= 4u)) {
        break;
      }
      a_1[v_9] = v_2((start_byte_offset + (v_9 * 16u)));
      {
        v_8 = (v_9 + 1u);
      }
    }
  }
  matrix<float16_t, 2, 3> v_10[4] = a_1;
  return v_10;
}

[numthreads(1, 1, 1)]
void f() {
  uint v_11 = (min(uint(i()), 3u) * 16u);
  uint v_12 = (min(uint(i()), 1u) * 8u);
  matrix<float16_t, 2, 3> l_a[4] = v_7(0u);
  matrix<float16_t, 2, 3> l_a_i = v_2(v_11);
  uint v_13 = (v_11 + v_12);
  uint4 v_14 = a[(v_13 / 16u)];
  vector<float16_t, 3> l_a_i_i = tint_bitcast_to_f16(select((((v_13 & 15u) >> 2u) == 2u), v_14.zw, v_14.xy)).xyz;
  uint v_15 = (v_11 + v_12);
  s.Store<float16_t>(0u, (((tint_bitcast_to_f16_1(a[(v_15 / 16u)][((v_15 & 15u) >> 2u)])[select(((v_15 % 4u) == 0u), 0u, 1u)] + l_a[0u][0u].x) + l_a_i[0u].x) + l_a_i_i.x));
}

