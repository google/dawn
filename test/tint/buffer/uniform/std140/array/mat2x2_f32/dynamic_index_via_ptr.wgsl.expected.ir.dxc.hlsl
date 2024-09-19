
cbuffer cbuffer_a : register(b0) {
  uint4 a[4];
};
RWByteAddressBuffer s : register(u1);
static int counter = int(0);
int i() {
  counter = (counter + int(1));
  return counter;
}

float2x2 v(uint start_byte_offset) {
  uint4 v_1 = a[(start_byte_offset / 16u)];
  float2 v_2 = asfloat((((((start_byte_offset % 16u) / 4u) == 2u)) ? (v_1.zw) : (v_1.xy)));
  uint4 v_3 = a[((8u + start_byte_offset) / 16u)];
  return float2x2(v_2, asfloat(((((((8u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_3.zw) : (v_3.xy))));
}

typedef float2x2 ary_ret[4];
ary_ret v_4(uint start_byte_offset) {
  float2x2 a[4] = (float2x2[4])0;
  {
    uint v_5 = 0u;
    v_5 = 0u;
    while(true) {
      uint v_6 = v_5;
      if ((v_6 >= 4u)) {
        break;
      }
      a[v_6] = v((start_byte_offset + (v_6 * 16u)));
      {
        v_5 = (v_6 + 1u);
      }
      continue;
    }
  }
  float2x2 v_7[4] = a;
  return v_7;
}

[numthreads(1, 1, 1)]
void f() {
  uint v_8 = (16u * uint(i()));
  uint v_9 = (8u * uint(i()));
  float2x2 v_10[4] = v_4(0u);
  float2x2 l_a_i = v(v_8);
  uint4 v_11 = a[((v_8 + v_9) / 16u)];
  float2 l_a_i_i = asfloat(((((((v_8 + v_9) % 16u) / 4u) == 2u)) ? (v_11.zw) : (v_11.xy)));
  float2x2 l_a[4] = v_10;
  s.Store(0u, asuint((((asfloat(a[((v_8 + v_9) / 16u)][(((v_8 + v_9) % 16u) / 4u)]) + l_a[int(0)][int(0)][0u]) + l_a_i[int(0)][0u]) + l_a_i_i[0u])));
}

