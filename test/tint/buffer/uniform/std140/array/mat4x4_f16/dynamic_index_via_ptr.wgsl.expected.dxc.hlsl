
cbuffer cbuffer_a : register(b0) {
  uint4 a[8];
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

matrix<float16_t, 4, 4> v_2(uint start_byte_offset) {
  uint4 v_3 = a[(start_byte_offset / 16u)];
  vector<float16_t, 4> v_4 = tint_bitcast_to_f16(select((((start_byte_offset & 15u) >> 2u) == 2u), v_3.zw, v_3.xy));
  uint v_5 = (8u + start_byte_offset);
  uint4 v_6 = a[(v_5 / 16u)];
  vector<float16_t, 4> v_7 = tint_bitcast_to_f16(select((((v_5 & 15u) >> 2u) == 2u), v_6.zw, v_6.xy));
  uint v_8 = (16u + start_byte_offset);
  uint4 v_9 = a[(v_8 / 16u)];
  vector<float16_t, 4> v_10 = tint_bitcast_to_f16(select((((v_8 & 15u) >> 2u) == 2u), v_9.zw, v_9.xy));
  uint v_11 = (24u + start_byte_offset);
  uint4 v_12 = a[(v_11 / 16u)];
  return matrix<float16_t, 4, 4>(v_4, v_7, v_10, tint_bitcast_to_f16(select((((v_11 & 15u) >> 2u) == 2u), v_12.zw, v_12.xy)));
}

typedef matrix<float16_t, 4, 4> ary_ret[4];
ary_ret v_13(uint start_byte_offset) {
  matrix<float16_t, 4, 4> a_1[4] = (matrix<float16_t, 4, 4>[4])0;
  {
    uint v_14 = 0u;
    v_14 = 0u;
    while(true) {
      uint v_15 = v_14;
      if ((v_15 >= 4u)) {
        break;
      }
      a_1[v_15] = v_2((start_byte_offset + (v_15 * 32u)));
      {
        v_14 = (v_15 + 1u);
      }
    }
  }
  matrix<float16_t, 4, 4> v_16[4] = a_1;
  return v_16;
}

[numthreads(1, 1, 1)]
void f() {
  uint v_17 = (min(uint(i()), 3u) * 32u);
  uint v_18 = (min(uint(i()), 3u) * 8u);
  matrix<float16_t, 4, 4> l_a[4] = v_13(0u);
  matrix<float16_t, 4, 4> l_a_i = v_2(v_17);
  uint v_19 = (v_17 + v_18);
  uint4 v_20 = a[(v_19 / 16u)];
  vector<float16_t, 4> l_a_i_i = tint_bitcast_to_f16(select((((v_19 & 15u) >> 2u) == 2u), v_20.zw, v_20.xy));
  uint v_21 = (v_17 + v_18);
  s.Store<float16_t>(0u, (((tint_bitcast_to_f16_1(a[(v_21 / 16u)][((v_21 & 15u) >> 2u)])[select(((v_21 % 4u) == 0u), 0u, 1u)] + l_a[0u][0u].x) + l_a_i[0u].x) + l_a_i_i.x));
}

