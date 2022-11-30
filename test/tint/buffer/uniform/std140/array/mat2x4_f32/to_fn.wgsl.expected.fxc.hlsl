cbuffer cbuffer_u : register(b0, space0) {
  uint4 u[8];
};

void a(float2x4 a_1[4]) {
}

void b(float2x4 m) {
}

void c(float4 v) {
}

void d(float f_1) {
}

float2x4 tint_symbol_1(uint4 buffer[8], uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  const uint scalar_offset_1 = ((offset + 16u)) / 4;
  return float2x4(asfloat(buffer[scalar_offset / 4]), asfloat(buffer[scalar_offset_1 / 4]));
}

typedef float2x4 tint_symbol_ret[4];
tint_symbol_ret tint_symbol(uint4 buffer[8], uint offset) {
  float2x4 arr[4] = (float2x4[4])0;
  {
    for(uint i = 0u; (i < 4u); i = (i + 1u)) {
      arr[i] = tint_symbol_1(buffer, (offset + (i * 32u)));
    }
  }
  return arr;
}

[numthreads(1, 1, 1)]
void f() {
  a(tint_symbol(u, 0u));
  b(tint_symbol_1(u, 32u));
  c(asfloat(u[2]).ywxz);
  d(asfloat(u[2]).ywxz.x);
  return;
}
