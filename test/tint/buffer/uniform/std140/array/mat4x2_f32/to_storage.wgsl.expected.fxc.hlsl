
cbuffer cbuffer_u : register(b0) {
  uint4 u[8];
};
RWByteAddressBuffer s : register(u1);
void v(uint offset, float4x2 obj) {
  s.Store2((offset + 0u), asuint(obj[0u]));
  s.Store2((offset + 8u), asuint(obj[1u]));
  s.Store2((offset + 16u), asuint(obj[2u]));
  s.Store2((offset + 24u), asuint(obj[3u]));
}

float4x2 v_1(uint start_byte_offset) {
  uint4 v_2 = u[(start_byte_offset / 16u)];
  float2 v_3 = asfloat((((((start_byte_offset & 15u) >> 2u) == 2u)) ? (v_2.zw) : (v_2.xy)));
  uint v_4 = (8u + start_byte_offset);
  uint4 v_5 = u[(v_4 / 16u)];
  float2 v_6 = asfloat((((((v_4 & 15u) >> 2u) == 2u)) ? (v_5.zw) : (v_5.xy)));
  uint v_7 = (16u + start_byte_offset);
  uint4 v_8 = u[(v_7 / 16u)];
  float2 v_9 = asfloat((((((v_7 & 15u) >> 2u) == 2u)) ? (v_8.zw) : (v_8.xy)));
  uint v_10 = (24u + start_byte_offset);
  uint4 v_11 = u[(v_10 / 16u)];
  return float4x2(v_3, v_6, v_9, asfloat((((((v_10 & 15u) >> 2u) == 2u)) ? (v_11.zw) : (v_11.xy))));
}

void v_12(uint offset, float4x2 obj[4]) {
  {
    uint idx = 0u;
    while(true) {
      uint v_13 = idx;
      if ((v_13 >= 4u)) {
        break;
      }
      v((offset + (v_13 * 32u)), obj[v_13]);
      {
        idx = (idx + 1u);
      }
    }
  }
}

typedef float4x2 ary_ret[4];
ary_ret v_14(uint start_byte_offset) {
  float4x2 a[4] = (float4x2[4])0;
  {
    uint idx = 0u;
    while(true) {
      uint v_15 = idx;
      if ((v_15 >= 4u)) {
        break;
      }
      a[v_15] = v_1((start_byte_offset + (v_15 * 32u)));
      {
        idx = (idx + 1u);
      }
    }
  }
  float4x2 v_16[4] = a;
  return v_16;
}

[numthreads(1, 1, 1)]
void f() {
  float4x2 v_17[4] = v_14(0u);
  v_12(0u, v_17);
  v(32u, v_1(64u));
  s.Store2(32u, asuint(asfloat(u[0u].zw).yx));
  s.Store(32u, asuint(asfloat(u[0u].z)));
}

