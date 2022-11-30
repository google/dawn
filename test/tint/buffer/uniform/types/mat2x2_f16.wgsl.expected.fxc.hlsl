SKIP: FAILED

cbuffer cbuffer_u : register(b0, space0) {
  uint4 u[1];
};

matrix<float16_t, 2, 2> tint_symbol(uint4 buffer[1], uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  uint ubo_load = buffer[scalar_offset / 4][scalar_offset % 4];
  const uint scalar_offset_1 = ((offset + 4u)) / 4;
  uint ubo_load_1 = buffer[scalar_offset_1 / 4][scalar_offset_1 % 4];
  return matrix<float16_t, 2, 2>(vector<float16_t, 2>(float16_t(f16tof32(ubo_load & 0xFFFF)), float16_t(f16tof32(ubo_load >> 16))), vector<float16_t, 2>(float16_t(f16tof32(ubo_load_1 & 0xFFFF)), float16_t(f16tof32(ubo_load_1 >> 16))));
}

[numthreads(1, 1, 1)]
void main() {
  const matrix<float16_t, 2, 2> x = tint_symbol(u, 0u);
  return;
}
FXC validation failure:
D:\Projects\RampUp\dawn\test\tint\buffer\Shader@0x0000017F74BE8F50(5,8-16): error X3000: syntax error: unexpected token 'float16_t'

