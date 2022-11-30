SKIP: FAILED

struct Inner {
  matrix<float16_t, 3, 4> m;
};
struct Outer {
  Inner a[4];
};

cbuffer cbuffer_a : register(b0, space0) {
  uint4 a[64];
};

matrix<float16_t, 3, 4> tint_symbol_4(uint4 buffer[64], uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  uint4 ubo_load_1 = buffer[scalar_offset / 4];
  uint2 ubo_load = ((scalar_offset & 2) ? ubo_load_1.zw : ubo_load_1.xy);
  vector<float16_t, 2> ubo_load_xz = vector<float16_t, 2>(f16tof32(ubo_load & 0xFFFF));
  vector<float16_t, 2> ubo_load_yw = vector<float16_t, 2>(f16tof32(ubo_load >> 16));
  const uint scalar_offset_1 = ((offset + 8u)) / 4;
  uint4 ubo_load_3 = buffer[scalar_offset_1 / 4];
  uint2 ubo_load_2 = ((scalar_offset_1 & 2) ? ubo_load_3.zw : ubo_load_3.xy);
  vector<float16_t, 2> ubo_load_2_xz = vector<float16_t, 2>(f16tof32(ubo_load_2 & 0xFFFF));
  vector<float16_t, 2> ubo_load_2_yw = vector<float16_t, 2>(f16tof32(ubo_load_2 >> 16));
  const uint scalar_offset_2 = ((offset + 16u)) / 4;
  uint4 ubo_load_5 = buffer[scalar_offset_2 / 4];
  uint2 ubo_load_4 = ((scalar_offset_2 & 2) ? ubo_load_5.zw : ubo_load_5.xy);
  vector<float16_t, 2> ubo_load_4_xz = vector<float16_t, 2>(f16tof32(ubo_load_4 & 0xFFFF));
  vector<float16_t, 2> ubo_load_4_yw = vector<float16_t, 2>(f16tof32(ubo_load_4 >> 16));
  return matrix<float16_t, 3, 4>(vector<float16_t, 4>(ubo_load_xz[0], ubo_load_yw[0], ubo_load_xz[1], ubo_load_yw[1]), vector<float16_t, 4>(ubo_load_2_xz[0], ubo_load_2_yw[0], ubo_load_2_xz[1], ubo_load_2_yw[1]), vector<float16_t, 4>(ubo_load_4_xz[0], ubo_load_4_yw[0], ubo_load_4_xz[1], ubo_load_4_yw[1]));
}

Inner tint_symbol_3(uint4 buffer[64], uint offset) {
  const Inner tint_symbol_7 = {tint_symbol_4(buffer, (offset + 0u))};
  return tint_symbol_7;
}

typedef Inner tint_symbol_2_ret[4];
tint_symbol_2_ret tint_symbol_2(uint4 buffer[64], uint offset) {
  Inner arr[4] = (Inner[4])0;
  {
    for(uint i = 0u; (i < 4u); i = (i + 1u)) {
      arr[i] = tint_symbol_3(buffer, (offset + (i * 64u)));
    }
  }
  return arr;
}

Outer tint_symbol_1(uint4 buffer[64], uint offset) {
  const Outer tint_symbol_8 = {tint_symbol_2(buffer, (offset + 0u))};
  return tint_symbol_8;
}

typedef Outer tint_symbol_ret[4];
tint_symbol_ret tint_symbol(uint4 buffer[64], uint offset) {
  Outer arr_1[4] = (Outer[4])0;
  {
    for(uint i_1 = 0u; (i_1 < 4u); i_1 = (i_1 + 1u)) {
      arr_1[i_1] = tint_symbol_1(buffer, (offset + (i_1 * 256u)));
    }
  }
  return arr_1;
}

[numthreads(1, 1, 1)]
void f() {
  const Outer l_a[4] = tint_symbol(a, 0u);
  const Outer l_a_3 = tint_symbol_1(a, 768u);
  const Inner l_a_3_a[4] = tint_symbol_2(a, 768u);
  const Inner l_a_3_a_2 = tint_symbol_3(a, 896u);
  const matrix<float16_t, 3, 4> l_a_3_a_2_m = tint_symbol_4(a, 896u);
  uint2 ubo_load_6 = a[56].zw;
  vector<float16_t, 2> ubo_load_6_xz = vector<float16_t, 2>(f16tof32(ubo_load_6 & 0xFFFF));
  vector<float16_t, 2> ubo_load_6_yw = vector<float16_t, 2>(f16tof32(ubo_load_6 >> 16));
  const vector<float16_t, 4> l_a_3_a_2_m_1 = vector<float16_t, 4>(ubo_load_6_xz[0], ubo_load_6_yw[0], ubo_load_6_xz[1], ubo_load_6_yw[1]);
  const float16_t l_a_3_a_2_m_1_0 = float16_t(f16tof32(((a[56].z) & 0xFFFF)));
  return;
}
FXC validation failure:
D:\Projects\RampUp\dawn\test\tint\buffer\Shader@0x000002CC6184FF70(2,10-18): error X3000: syntax error: unexpected token 'float16_t'

