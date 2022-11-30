SKIP: FAILED

struct S {
  int before;
  matrix<float16_t, 3, 3> m;
  int after;
};

cbuffer cbuffer_u : register(b0, space0) {
  uint4 u[32];
};
static S p[4] = (S[4])0;

matrix<float16_t, 3, 3> tint_symbol_3(uint4 buffer[32], uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  uint4 ubo_load_1 = buffer[scalar_offset / 4];
  uint2 ubo_load = ((scalar_offset & 2) ? ubo_load_1.zw : ubo_load_1.xy);
  vector<float16_t, 2> ubo_load_xz = vector<float16_t, 2>(f16tof32(ubo_load & 0xFFFF));
  float16_t ubo_load_y = f16tof32(ubo_load[0] >> 16);
  const uint scalar_offset_1 = ((offset + 8u)) / 4;
  uint4 ubo_load_3 = buffer[scalar_offset_1 / 4];
  uint2 ubo_load_2 = ((scalar_offset_1 & 2) ? ubo_load_3.zw : ubo_load_3.xy);
  vector<float16_t, 2> ubo_load_2_xz = vector<float16_t, 2>(f16tof32(ubo_load_2 & 0xFFFF));
  float16_t ubo_load_2_y = f16tof32(ubo_load_2[0] >> 16);
  const uint scalar_offset_2 = ((offset + 16u)) / 4;
  uint4 ubo_load_5 = buffer[scalar_offset_2 / 4];
  uint2 ubo_load_4 = ((scalar_offset_2 & 2) ? ubo_load_5.zw : ubo_load_5.xy);
  vector<float16_t, 2> ubo_load_4_xz = vector<float16_t, 2>(f16tof32(ubo_load_4 & 0xFFFF));
  float16_t ubo_load_4_y = f16tof32(ubo_load_4[0] >> 16);
  return matrix<float16_t, 3, 3>(vector<float16_t, 3>(ubo_load_xz[0], ubo_load_y, ubo_load_xz[1]), vector<float16_t, 3>(ubo_load_2_xz[0], ubo_load_2_y, ubo_load_2_xz[1]), vector<float16_t, 3>(ubo_load_4_xz[0], ubo_load_4_y, ubo_load_4_xz[1]));
}

S tint_symbol_1(uint4 buffer[32], uint offset) {
  const uint scalar_offset_3 = ((offset + 0u)) / 4;
  const uint scalar_offset_4 = ((offset + 64u)) / 4;
  const S tint_symbol_5 = {asint(buffer[scalar_offset_3 / 4][scalar_offset_3 % 4]), tint_symbol_3(buffer, (offset + 8u)), asint(buffer[scalar_offset_4 / 4][scalar_offset_4 % 4])};
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
  uint2 ubo_load_6 = u[1].xy;
  vector<float16_t, 2> ubo_load_6_xz = vector<float16_t, 2>(f16tof32(ubo_load_6 & 0xFFFF));
  float16_t ubo_load_6_y = f16tof32(ubo_load_6[0] >> 16);
  p[1].m[0] = vector<float16_t, 3>(ubo_load_6_xz[0], ubo_load_6_y, ubo_load_6_xz[1]).zxy;
  return;
}
FXC validation failure:
D:\Projects\RampUp\dawn\test\tint\buffer\Shader@0x0000025A3E276990(3,10-18): error X3000: syntax error: unexpected token 'float16_t'

