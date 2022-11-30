struct S {
  int before;
  float4x3 m;
  int after;
};

cbuffer cbuffer_u : register(b0, space0) {
  uint4 u[48];
};
static S p[4] = (S[4])0;

float4x3 tint_symbol_3(uint4 buffer[48], uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  const uint scalar_offset_1 = ((offset + 16u)) / 4;
  const uint scalar_offset_2 = ((offset + 32u)) / 4;
  const uint scalar_offset_3 = ((offset + 48u)) / 4;
  return float4x3(asfloat(buffer[scalar_offset / 4].xyz), asfloat(buffer[scalar_offset_1 / 4].xyz), asfloat(buffer[scalar_offset_2 / 4].xyz), asfloat(buffer[scalar_offset_3 / 4].xyz));
}

S tint_symbol_1(uint4 buffer[48], uint offset) {
  const uint scalar_offset_4 = ((offset + 0u)) / 4;
  const uint scalar_offset_5 = ((offset + 128u)) / 4;
  const S tint_symbol_5 = {asint(buffer[scalar_offset_4 / 4][scalar_offset_4 % 4]), tint_symbol_3(buffer, (offset + 16u)), asint(buffer[scalar_offset_5 / 4][scalar_offset_5 % 4])};
  return tint_symbol_5;
}

typedef S tint_symbol_ret[4];
tint_symbol_ret tint_symbol(uint4 buffer[48], uint offset) {
  S arr[4] = (S[4])0;
  {
    for(uint i = 0u; (i < 4u); i = (i + 1u)) {
      arr[i] = tint_symbol_1(buffer, (offset + (i * 192u)));
    }
  }
  return arr;
}

[numthreads(1, 1, 1)]
void f() {
  p = tint_symbol(u, 0u);
  p[1] = tint_symbol_1(u, 384u);
  p[3].m = tint_symbol_3(u, 400u);
  p[1].m[0] = asfloat(u[2].xyz).zxy;
  return;
}
