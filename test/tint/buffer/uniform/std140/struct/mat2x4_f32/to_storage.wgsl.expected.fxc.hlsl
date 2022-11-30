struct S {
  int before;
  float2x4 m;
  int after;
};

cbuffer cbuffer_u : register(b0, space0) {
  uint4 u[32];
};
RWByteAddressBuffer s : register(u1, space0);

void tint_symbol_3(RWByteAddressBuffer buffer, uint offset, float2x4 value) {
  buffer.Store4((offset + 0u), asuint(value[0u]));
  buffer.Store4((offset + 16u), asuint(value[1u]));
}

void tint_symbol_1(RWByteAddressBuffer buffer, uint offset, S value) {
  buffer.Store((offset + 0u), asuint(value.before));
  tint_symbol_3(buffer, (offset + 16u), value.m);
  buffer.Store((offset + 64u), asuint(value.after));
}

void tint_symbol(RWByteAddressBuffer buffer, uint offset, S value[4]) {
  S array[4] = value;
  {
    for(uint i = 0u; (i < 4u); i = (i + 1u)) {
      tint_symbol_1(buffer, (offset + (i * 128u)), array[i]);
    }
  }
}

float2x4 tint_symbol_8(uint4 buffer[32], uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  const uint scalar_offset_1 = ((offset + 16u)) / 4;
  return float2x4(asfloat(buffer[scalar_offset / 4]), asfloat(buffer[scalar_offset_1 / 4]));
}

S tint_symbol_6(uint4 buffer[32], uint offset) {
  const uint scalar_offset_2 = ((offset + 0u)) / 4;
  const uint scalar_offset_3 = ((offset + 64u)) / 4;
  const S tint_symbol_10 = {asint(buffer[scalar_offset_2 / 4][scalar_offset_2 % 4]), tint_symbol_8(buffer, (offset + 16u)), asint(buffer[scalar_offset_3 / 4][scalar_offset_3 % 4])};
  return tint_symbol_10;
}

typedef S tint_symbol_5_ret[4];
tint_symbol_5_ret tint_symbol_5(uint4 buffer[32], uint offset) {
  S arr[4] = (S[4])0;
  {
    for(uint i_1 = 0u; (i_1 < 4u); i_1 = (i_1 + 1u)) {
      arr[i_1] = tint_symbol_6(buffer, (offset + (i_1 * 128u)));
    }
  }
  return arr;
}

[numthreads(1, 1, 1)]
void f() {
  tint_symbol(s, 0u, tint_symbol_5(u, 0u));
  tint_symbol_1(s, 128u, tint_symbol_6(u, 256u));
  tint_symbol_3(s, 400u, tint_symbol_8(u, 272u));
  s.Store4(144u, asuint(asfloat(u[2]).ywxz));
  return;
}
