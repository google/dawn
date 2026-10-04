
cbuffer cbuffer_u : register(b0) {
  uint4 u[4];
};
RWByteAddressBuffer s : register(u1);
void v(uint offset, float2x2 obj) {
  s.Store2((offset + 0u), asuint(obj[0u]));
  s.Store2((offset + 8u), asuint(obj[1u]));
}

float2x2 v_1(uint start_byte_offset) {
  uint4 v_2 = u[(start_byte_offset / 16u)];
  float2 v_3 = asfloat((((((start_byte_offset & 15u) >> 2u) == 2u)) ? (v_2.zw) : (v_2.xy)));
  uint v_4 = (8u + start_byte_offset);
  uint4 v_5 = u[(v_4 / 16u)];
  return float2x2(v_3, asfloat((((((v_4 & 15u) >> 2u) == 2u)) ? (v_5.zw) : (v_5.xy))));
}

void v_6(uint offset, float2x2 obj[4]) {
  {
    uint idx = 0u;
    while(true) {
      uint v_7 = idx;
      if ((v_7 >= 4u)) {
        break;
      }
      v((offset + (v_7 * 16u)), obj[v_7]);
      {
        idx = (idx + 1u);
      }
    }
  }
}

typedef float2x2 ary_ret[4];
ary_ret v_8(uint start_byte_offset) {
  float2x2 a[4] = (float2x2[4])0;
  {
    uint idx = 0u;
    while(true) {
      uint v_9 = idx;
      if ((v_9 >= 4u)) {
        break;
      }
      a[v_9] = v_1((start_byte_offset + (v_9 * 16u)));
      {
        idx = (idx + 1u);
      }
    }
  }
  float2x2 v_10[4] = a;
  return v_10;
}

[numthreads(1, 1, 1)]
void f() {
  float2x2 v_11[4] = v_8(0u);
  v_6(0u, v_11);
  v(16u, v_1(32u));
  s.Store2(16u, asuint(asfloat(u[0u].zw).yx));
  s.Store(16u, asuint(asfloat(u[0u].z)));
}

