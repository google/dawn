cbuffer cbuffer_u : register(b0, space0) {
  uint4 u[12];
};

void a(float3x4 a_1[4]) {
}

void b(float3x4 m) {
}

void c(float4 v) {
}

void d(float f_1) {
}

float3x4 tint_symbol_1(uint4 buffer[12], uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  const uint scalar_offset_1 = ((offset + 16u)) / 4;
  const uint scalar_offset_2 = ((offset + 32u)) / 4;
  return float3x4(asfloat(buffer[scalar_offset / 4]), asfloat(buffer[scalar_offset_1 / 4]), asfloat(buffer[scalar_offset_2 / 4]));
}

typedef float3x4 tint_symbol_ret[4];
tint_symbol_ret tint_symbol(uint4 buffer[12], uint offset) {
  float3x4 arr[4] = (float3x4[4])0;
  {
    for(uint i = 0u; (i < 4u); i = (i + 1u)) {
      arr[i] = tint_symbol_1(buffer, (offset + (i * 48u)));
    }
  }
  return arr;
}

[numthreads(1, 1, 1)]
void f() {
  a(tint_symbol(u, 0u));
  b(tint_symbol_1(u, 48u));
  c(asfloat(u[3]).ywxz);
  d(asfloat(u[3]).ywxz.x);
  return;
}
