
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
  uint v_3 = (8u + start_byte_offset);
  uint4 v_4 = u[(v_3 / 16u)];
  return float2x2(asfloat(select((((start_byte_offset & 15u) >> 2u) == 2u), v_2.zw, v_2.xy)), asfloat(select((((v_3 & 15u) >> 2u) == 2u), v_4.zw, v_4.xy)));
}

void v_5(uint offset, float2x2 obj[4]) {
  {
    uint idx = 0u;
    while(true) {
      uint v_6 = idx;
      if ((v_6 >= 4u)) {
        break;
      }
      v((offset + (v_6 * 16u)), obj[v_6]);
      {
        idx = (idx + 1u);
      }
    }
  }
}

typedef float2x2 ary_ret[4];
ary_ret v_7(uint start_byte_offset) {
  float2x2 a[4] = (float2x2[4])0;
  {
    uint idx = 0u;
    while(true) {
      uint v_8 = idx;
      if ((v_8 >= 4u)) {
        break;
      }
      a[v_8] = v_1((start_byte_offset + (v_8 * 16u)));
      {
        idx = (idx + 1u);
      }
    }
  }
  float2x2 v_9[4] = a;
  return v_9;
}

[numthreads(1, 1, 1)]
void f() {
  float2x2 v_10[4] = v_7(0u);
  v_5(0u, v_10);
  v(16u, v_1(32u));
  s.Store2(16u, asuint(asfloat(u[0u].zw).yx));
  s.Store(16u, asuint(asfloat(u[0u].z)));
}

