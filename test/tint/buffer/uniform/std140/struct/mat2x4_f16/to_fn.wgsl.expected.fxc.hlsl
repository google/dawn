SKIP: FAILED

struct S {
  int before;
  matrix<float16_t, 2, 4> m;
  int after;
};

cbuffer cbuffer_u : register(b0, space0) {
  uint4 u[32];
};

void a(S a_1[4]) {
}

void b(S s) {
}

void c(matrix<float16_t, 2, 4> m) {
}

void d(vector<float16_t, 4> v) {
}

void e(float16_t f_1) {
}

matrix<float16_t, 2, 4> tint_symbol_3(uint4 buffer[32], uint offset) {
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
  return matrix<float16_t, 2, 4>(vector<float16_t, 4>(ubo_load_xz[0], ubo_load_yw[0], ubo_load_xz[1], ubo_load_yw[1]), vector<float16_t, 4>(ubo_load_2_xz[0], ubo_load_2_yw[0], ubo_load_2_xz[1], ubo_load_2_yw[1]));
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
  a(tint_symbol(u, 0u));
  b(tint_symbol_1(u, 256u));
  c(tint_symbol_3(u, 264u));
  uint2 ubo_load_4 = u[1].xy;
  vector<float16_t, 2> ubo_load_4_xz = vector<float16_t, 2>(f16tof32(ubo_load_4 & 0xFFFF));
  vector<float16_t, 2> ubo_load_4_yw = vector<float16_t, 2>(f16tof32(ubo_load_4 >> 16));
  d(vector<float16_t, 4>(ubo_load_4_xz[0], ubo_load_4_yw[0], ubo_load_4_xz[1], ubo_load_4_yw[1]).ywxz);
  uint2 ubo_load_5 = u[1].xy;
  vector<float16_t, 2> ubo_load_5_xz = vector<float16_t, 2>(f16tof32(ubo_load_5 & 0xFFFF));
  vector<float16_t, 2> ubo_load_5_yw = vector<float16_t, 2>(f16tof32(ubo_load_5 >> 16));
  e(vector<float16_t, 4>(ubo_load_5_xz[0], ubo_load_5_yw[0], ubo_load_5_xz[1], ubo_load_5_yw[1]).ywxz.x);
  return;
}
FXC validation failure:
D:\Projects\RampUp\dawn\test\tint\buffer\Shader@0x000001B118E73590(3,10-18): error X3000: syntax error: unexpected token 'float16_t'

