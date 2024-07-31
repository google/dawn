
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
  float3 v_2 = asfloat(u[(start_byte_offset / 16u)].xyz);
  float3 v_3 = asfloat(u[((16u + start_byte_offset) / 16u)].xyz);
  return float3x3(v_2, v_3, asfloat(u[((32u + start_byte_offset) / 16u)].xyz));
}

void v_4(uint offset, float3x3 obj[4]) {
  {
    uint v_5 = 0u;
    v_5 = 0u;
    while(true) {
      uint v_6 = v_5;
      if ((v_6 >= 4u)) {
        break;
      }
      v((offset + (v_6 * 48u)), obj[v_6]);
      {
        v_5 = (v_6 + 1u);
      }
      continue;
    }
  }
}

typedef float3x3 ary_ret[4];
ary_ret v_7(uint start_byte_offset) {
  float3x3 a[4] = (float3x3[4])0;
  {
    uint v_8 = 0u;
    v_8 = 0u;
    while(true) {
      uint v_9 = v_8;
      if ((v_9 >= 4u)) {
        break;
      }
      a[v_9] = v_1((start_byte_offset + (v_9 * 48u)));
      {
        v_8 = (v_9 + 1u);
      }
      continue;
    }
  }
  float3x3 v_10[4] = a;
  return v_10;
}

[numthreads(1, 1, 1)]
void f() {
  float3x3 v_11[4] = v_7(0u);
  v_4(0u, v_11);
  v(48u, v_1(96u));
  s.Store3(48u, asuint(asfloat(u[1u].xyz).zxy));
  s.Store(48u, asuint(asfloat(u[1u].x)));
}

