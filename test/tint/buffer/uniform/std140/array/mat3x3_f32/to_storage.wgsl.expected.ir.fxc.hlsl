
cbuffer cbuffer_u : register(b0) {
  uint4 u[12];
};
RWByteAddressBuffer s : register(u1);
void v(uint offset, float3x3 obj) {
  s.Store3((offset + 0u), asuint(obj[0u]));
  s.Store3((offset + 16u), asuint(obj[1u]));
  s.Store3((offset + 32u), asuint(obj[2u]));
}

float3x3 v_1(uint start_byte_offset) {
  return float3x3(asfloat(u[(start_byte_offset / 16u)].xyz), asfloat(u[((16u + start_byte_offset) / 16u)].xyz), asfloat(u[((32u + start_byte_offset) / 16u)].xyz));
}

void v_2(uint offset, float3x3 obj[4]) {
  {
    uint v_3 = 0u;
    v_3 = 0u;
    while(true) {
      uint v_4 = v_3;
      if ((v_4 >= 4u)) {
        break;
      }
      v((offset + (v_4 * 48u)), obj[v_4]);
      {
        v_3 = (v_4 + 1u);
      }
      continue;
    }
  }
}

typedef float3x3 ary_ret[4];
ary_ret v_5(uint start_byte_offset) {
  float3x3 a[4] = (float3x3[4])0;
  {
    uint v_6 = 0u;
    v_6 = 0u;
    while(true) {
      uint v_7 = v_6;
      if ((v_7 >= 4u)) {
        break;
      }
      a[v_7] = v_1((start_byte_offset + (v_7 * 48u)));
      {
        v_6 = (v_7 + 1u);
      }
      continue;
    }
  }
  float3x3 v_8[4] = a;
  return v_8;
}

[numthreads(1, 1, 1)]
void f() {
  float3x3 v_9[4] = v_5(0u);
  v_2(0u, v_9);
  v(48u, v_1(96u));
  s.Store3(48u, asuint(asfloat(u[1u].xyz).zxy));
  s.Store(48u, asuint(asfloat(u[1u].x)));
}

