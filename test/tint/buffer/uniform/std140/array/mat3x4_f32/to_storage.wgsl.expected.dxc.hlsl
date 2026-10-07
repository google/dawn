
cbuffer cbuffer_u : register(b0) {
  uint4 u[12];
};
RWByteAddressBuffer s : register(u1);
void v(uint offset, float3x4 obj) {
  s.Store4((offset + 0u), asuint(obj[0u]));
  s.Store4((offset + 16u), asuint(obj[1u]));
  s.Store4((offset + 32u), asuint(obj[2u]));
}

float3x4 v_1(uint start_byte_offset) {
  return float3x4(asfloat(u[(start_byte_offset / 16u)]), asfloat(u[((16u + start_byte_offset) / 16u)]), asfloat(u[((32u + start_byte_offset) / 16u)]));
}

void v_2(uint offset, float3x4 obj[4]) {
  {
    uint idx = 0u;
    while(true) {
      uint v_3 = idx;
      if ((v_3 >= 4u)) {
        break;
      }
      v((offset + (v_3 * 48u)), obj[v_3]);
      {
        idx = (idx + 1u);
      }
    }
  }
}

typedef float3x4 ary_ret[4];
ary_ret v_4(uint start_byte_offset) {
  float3x4 a[4] = (float3x4[4])0;
  {
    uint idx = 0u;
    while(true) {
      uint v_5 = idx;
      if ((v_5 >= 4u)) {
        break;
      }
      a[v_5] = v_1((start_byte_offset + (v_5 * 48u)));
      {
        idx = (idx + 1u);
      }
    }
  }
  float3x4 v_6[4] = a;
  return v_6;
}

[numthreads(1, 1, 1)]
void f() {
  float3x4 v_7[4] = v_4(0u);
  v_2(0u, v_7);
  v(48u, v_1(96u));
  s.Store4(48u, asuint(asfloat(u[1u]).ywxz));
  s.Store(48u, asuint(asfloat(u[1u].x)));
}

