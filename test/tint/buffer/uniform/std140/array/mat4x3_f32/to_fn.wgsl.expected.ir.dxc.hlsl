
cbuffer cbuffer_u : register(b0) {
  uint4 u[16];
};
RWByteAddressBuffer s : register(u1);
float a(float4x3 a_1[4]) {
  return a_1[int(0)][int(0)][0u];
}

float b(float4x3 m) {
  return m[int(0)][0u];
}

float c(float3 v) {
  return v[0u];
}

float d(float f_1) {
  return f_1;
}

float4x3 v_1(uint start_byte_offset) {
  float3 v_2 = asfloat(u[(start_byte_offset / 16u)].xyz);
  float3 v_3 = asfloat(u[((16u + start_byte_offset) / 16u)].xyz);
  float3 v_4 = asfloat(u[((32u + start_byte_offset) / 16u)].xyz);
  return float4x3(v_2, v_3, v_4, asfloat(u[((48u + start_byte_offset) / 16u)].xyz));
}

typedef float4x3 ary_ret[4];
ary_ret v_5(uint start_byte_offset) {
  float4x3 a_2[4] = (float4x3[4])0;
  {
    uint v_6 = 0u;
    v_6 = 0u;
    while(true) {
      uint v_7 = v_6;
      if ((v_7 >= 4u)) {
        break;
      }
      a_2[v_7] = v_1((start_byte_offset + (v_7 * 64u)));
      {
        v_6 = (v_7 + 1u);
      }
      continue;
    }
  }
  float4x3 v_8[4] = a_2;
  return v_8;
}

[numthreads(1, 1, 1)]
void f() {
  float4x3 v_9[4] = v_5(0u);
  float v_10 = a(v_9);
  float v_11 = (v_10 + b(v_1(64u)));
  float v_12 = (v_11 + c(asfloat(u[4u].xyz).zxy));
  s.Store(0u, asuint((v_12 + d(asfloat(u[4u].xyz).zxy[0u]))));
}

