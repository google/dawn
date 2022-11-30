struct S {
  int before;
  float2x2 m;
  int after;
};

cbuffer cbuffer_u : register(b0, space0) {
  uint4 u[32];
};
static S p[4] = (S[4])0;

float2x2 tint_symbol_3(uint4 buffer[32], uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  uint4 ubo_load = buffer[scalar_offset / 4];
  const uint scalar_offset_1 = ((offset + 8u)) / 4;
  uint4 ubo_load_1 = buffer[scalar_offset_1 / 4];
  return float2x2(asfloat(((scalar_offset & 2) ? ubo_load.zw : ubo_load.xy)), asfloat(((scalar_offset_1 & 2) ? ubo_load_1.zw : ubo_load_1.xy)));
}

S tint_symbol_1(uint4 buffer[32], uint offset) {
  const uint scalar_offset_2 = ((offset + 0u)) / 4;
  const uint scalar_offset_3 = ((offset + 64u)) / 4;
  const S tint_symbol_5 = {asint(buffer[scalar_offset_2 / 4][scalar_offset_2 % 4]), tint_symbol_3(buffer, (offset + 8u)), asint(buffer[scalar_offset_3 / 4][scalar_offset_3 % 4])};
  return tint_symbol_5;
}

typedef S tint_symbol_ret[4];
tint_symbol_ret tint_symbol(uint4 buffer[32], uint offset) {
  S arr[4] = (S[4])0;
  {
    for(uint i = 0u; (i < 4u); i = (i + 1u)) {
      arr[i] = tint_symbol_1(buffer, (offset + (i * 128u)));
    }
  }
  return arr;
}

[numthreads(1, 1, 1)]
void f() {
  p = tint_symbol(u, 0u);
  p[1] = tint_symbol_1(u, 256u);
  p[3].m = tint_symbol_3(u, 264u);
  p[1].m[0] = asfloat(u[1].xy).yx;
  return;
}
