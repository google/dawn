struct S {
  int before;
  float4x2 m;
  int after;
};

cbuffer cbuffer_u : register(b0, space0) {
  uint4 u[12];
};

void a(S a_1[4]) {
}

void b(S s) {
}

void c(float4x2 m) {
}

void d(float2 v) {
}

void e(float f_1) {
}

float4x2 tint_symbol_3(uint4 buffer[12], uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  uint4 ubo_load = buffer[scalar_offset / 4];
  const uint scalar_offset_1 = ((offset + 8u)) / 4;
  uint4 ubo_load_1 = buffer[scalar_offset_1 / 4];
  const uint scalar_offset_2 = ((offset + 16u)) / 4;
  uint4 ubo_load_2 = buffer[scalar_offset_2 / 4];
  const uint scalar_offset_3 = ((offset + 24u)) / 4;
  uint4 ubo_load_3 = buffer[scalar_offset_3 / 4];
  return float4x2(asfloat(((scalar_offset & 2) ? ubo_load.zw : ubo_load.xy)), asfloat(((scalar_offset_1 & 2) ? ubo_load_1.zw : ubo_load_1.xy)), asfloat(((scalar_offset_2 & 2) ? ubo_load_2.zw : ubo_load_2.xy)), asfloat(((scalar_offset_3 & 2) ? ubo_load_3.zw : ubo_load_3.xy)));
}

S tint_symbol_1(uint4 buffer[12], uint offset) {
  const uint scalar_offset_4 = ((offset + 0u)) / 4;
  const uint scalar_offset_5 = ((offset + 40u)) / 4;
  const S tint_symbol_5 = {asint(buffer[scalar_offset_4 / 4][scalar_offset_4 % 4]), tint_symbol_3(buffer, (offset + 8u)), asint(buffer[scalar_offset_5 / 4][scalar_offset_5 % 4])};
  return tint_symbol_5;
}

typedef S tint_symbol_ret[4];
tint_symbol_ret tint_symbol(uint4 buffer[12], uint offset) {
  S arr[4] = (S[4])0;
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
  b(tint_symbol_1(u, 96u));
  c(tint_symbol_3(u, 104u));
  d(asfloat(u[1].xy).yx);
  e(asfloat(u[1].xy).yx.x);
  return;
}
