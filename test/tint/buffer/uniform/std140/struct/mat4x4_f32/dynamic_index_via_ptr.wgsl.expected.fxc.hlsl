struct Inner {
  float4x4 m;
};
struct Outer {
  Inner a[4];
};

cbuffer cbuffer_a : register(b0, space0) {
  uint4 a[64];
};
static int counter = 0;

int i() {
  counter = (counter + 1);
  return counter;
}

float4x4 tint_symbol_8(uint4 buffer[64], uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  const uint scalar_offset_1 = ((offset + 16u)) / 4;
  const uint scalar_offset_2 = ((offset + 32u)) / 4;
  const uint scalar_offset_3 = ((offset + 48u)) / 4;
  return float4x4(asfloat(buffer[scalar_offset / 4]), asfloat(buffer[scalar_offset_1 / 4]), asfloat(buffer[scalar_offset_2 / 4]), asfloat(buffer[scalar_offset_3 / 4]));
}

Inner tint_symbol_7(uint4 buffer[64], uint offset) {
  const Inner tint_symbol_11 = {tint_symbol_8(buffer, (offset + 0u))};
  return tint_symbol_11;
}

typedef Inner tint_symbol_6_ret[4];
tint_symbol_6_ret tint_symbol_6(uint4 buffer[64], uint offset) {
  Inner arr[4] = (Inner[4])0;
  {
    for(uint i_1 = 0u; (i_1 < 4u); i_1 = (i_1 + 1u)) {
      arr[i_1] = tint_symbol_7(buffer, (offset + (i_1 * 64u)));
    }
  }
  return arr;
}

Outer tint_symbol_5(uint4 buffer[64], uint offset) {
  const Outer tint_symbol_12 = {tint_symbol_6(buffer, (offset + 0u))};
  return tint_symbol_12;
}

typedef Outer tint_symbol_4_ret[4];
tint_symbol_4_ret tint_symbol_4(uint4 buffer[64], uint offset) {
  Outer arr_1[4] = (Outer[4])0;
  {
    for(uint i_2 = 0u; (i_2 < 4u); i_2 = (i_2 + 1u)) {
      arr_1[i_2] = tint_symbol_5(buffer, (offset + (i_2 * 256u)));
    }
  }
  return arr_1;
}

[numthreads(1, 1, 1)]
void f() {
  const int p_a_i_save = i();
  const int p_a_i_a_i_save = i();
  const int p_a_i_a_i_m_i_save = i();
  const Outer l_a[4] = tint_symbol_4(a, 0u);
  const Outer l_a_i = tint_symbol_5(a, (256u * uint(p_a_i_save)));
  const Inner l_a_i_a[4] = tint_symbol_6(a, (256u * uint(p_a_i_save)));
  const Inner l_a_i_a_i = tint_symbol_7(a, ((256u * uint(p_a_i_save)) + (64u * uint(p_a_i_a_i_save))));
  const float4x4 l_a_i_a_i_m = tint_symbol_8(a, ((256u * uint(p_a_i_save)) + (64u * uint(p_a_i_a_i_save))));
  const uint scalar_offset_4 = ((((256u * uint(p_a_i_save)) + (64u * uint(p_a_i_a_i_save))) + (16u * uint(p_a_i_a_i_m_i_save)))) / 4;
  const float4 l_a_i_a_i_m_i = asfloat(a[scalar_offset_4 / 4]);
  const int tint_symbol = p_a_i_save;
  const int tint_symbol_1 = p_a_i_a_i_save;
  const int tint_symbol_2 = p_a_i_a_i_m_i_save;
  const int tint_symbol_3 = i();
  const uint scalar_offset_5 = (((((256u * uint(tint_symbol)) + (64u * uint(tint_symbol_1))) + (16u * uint(tint_symbol_2))) + (4u * uint(tint_symbol_3)))) / 4;
  const float l_a_i_a_i_m_i_i = asfloat(a[scalar_offset_5 / 4][scalar_offset_5 % 4]);
  return;
}
