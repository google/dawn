cbuffer cbuffer_u : register(b0) {
  uint4 u[16];
};
RWByteAddressBuffer s : register(u1);

float a(float4x3 a_1[4]) {
  return a_1[0][0].x;
}

float b(float4x3 m) {
  return m[0].x;
}

float c(float3 v) {
  return v.x;
}

float d(float f_1) {
  return f_1;
}

float4x3 u_load_1(uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  const uint scalar_offset_1 = ((offset + 16u)) / 4;
  const uint scalar_offset_2 = ((offset + 32u)) / 4;
  const uint scalar_offset_3 = ((offset + 48u)) / 4;
  return float4x3(asfloat(u[scalar_offset / 4].xyz), asfloat(u[scalar_offset_1 / 4].xyz), asfloat(u[scalar_offset_2 / 4].xyz), asfloat(u[scalar_offset_3 / 4].xyz));
}

typedef float4x3 u_load_ret[4];
u_load_ret u_load(uint offset) {
  float4x3 arr[4] = (float4x3[4])0;
  {
    for(uint i = 0u; (i < 4u); i = (i + 1u)) {
      arr[i] = u_load_1((offset + (i * 64u)));
    }
  }
  return arr;
}

[numthreads(1, 1, 1)]
void f() {
  float tint_symbol = a(u_load(0u));
  float tint_symbol_1 = b(u_load_1(64u));
  float tint_symbol_2 = c(asfloat(u[4].xyz).zxy);
  float tint_symbol_3 = d(asfloat(u[4].xyz).zxy.x);
  s.Store(0u, asuint((((tint_symbol + tint_symbol_1) + tint_symbol_2) + tint_symbol_3)));
  return;
}
