
cbuffer cbuffer_u : register(b0) {
  uint4 u[8];
};
RWByteAddressBuffer s : register(u1);
float a(float2x4 a_1[4]) {
  return a_1[int(0)][int(0)][0u];
}

float b(float2x4 m) {
  return m[int(0)][0u];
}

float c(float4 v) {
  return v[0u];
}

float d(float f) {
  return f;
}

float2x4 v_1(uint start_byte_offset) {
  float4 v_2 = asfloat(u[(start_byte_offset / 16u)]);
  return float2x4(v_2, asfloat(u[((16u + start_byte_offset) / 16u)]));
}

typedef float2x4 ary_ret[4];
ary_ret v_3(uint start_byte_offset) {
  float2x4 a[4] = (float2x4[4])0;
  {
    uint v_4 = 0u;
    v_4 = 0u;
    while(true) {
      uint v_5 = v_4;
      if ((v_5 >= 4u)) {
        break;
      }
      a[v_5] = v_1((start_byte_offset + (v_5 * 32u)));
      {
        v_4 = (v_5 + 1u);
      }
      continue;
    }
  }
  float2x4 v_6[4] = a;
  return v_6;
}

[numthreads(1, 1, 1)]
void f() {
  float2x4 v_7[4] = v_3(0u);
  float v_8 = a(v_7);
  float v_9 = (v_8 + b(v_1(32u)));
  float v_10 = (v_9 + c(asfloat(u[2u]).ywxz));
  s.Store(0u, asuint((v_10 + d(asfloat(u[2u]).ywxz[0u]))));
}

