SKIP: FAILED

cbuffer cbuffer_u : register(b0, space0) {
  uint4 u[1];
};
static matrix<float16_t, 4, 2> p = matrix<float16_t, 4, 2>(float16_t(0.0h), float16_t(0.0h), float16_t(0.0h), float16_t(0.0h), float16_t(0.0h), float16_t(0.0h), float16_t(0.0h), float16_t(0.0h));

matrix<float16_t, 4, 2> tint_symbol(uint4 buffer[1], uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  uint ubo_load = buffer[scalar_offset / 4][scalar_offset % 4];
  const uint scalar_offset_1 = ((offset + 4u)) / 4;
  uint ubo_load_1 = buffer[scalar_offset_1 / 4][scalar_offset_1 % 4];
  const uint scalar_offset_2 = ((offset + 8u)) / 4;
  uint ubo_load_2 = buffer[scalar_offset_2 / 4][scalar_offset_2 % 4];
  const uint scalar_offset_3 = ((offset + 12u)) / 4;
  uint ubo_load_3 = buffer[scalar_offset_3 / 4][scalar_offset_3 % 4];
  return matrix<float16_t, 4, 2>(vector<float16_t, 2>(float16_t(f16tof32(ubo_load & 0xFFFF)), float16_t(f16tof32(ubo_load >> 16))), vector<float16_t, 2>(float16_t(f16tof32(ubo_load_1 & 0xFFFF)), float16_t(f16tof32(ubo_load_1 >> 16))), vector<float16_t, 2>(float16_t(f16tof32(ubo_load_2 & 0xFFFF)), float16_t(f16tof32(ubo_load_2 >> 16))), vector<float16_t, 2>(float16_t(f16tof32(ubo_load_3 & 0xFFFF)), float16_t(f16tof32(ubo_load_3 >> 16))));
}

[numthreads(1, 1, 1)]
void f() {
  p = tint_symbol(u, 0u);
  uint ubo_load_4 = u[0].x;
  p[1] = vector<float16_t, 2>(float16_t(f16tof32(ubo_load_4 & 0xFFFF)), float16_t(f16tof32(ubo_load_4 >> 16)));
  uint ubo_load_5 = u[0].x;
  p[1] = vector<float16_t, 2>(float16_t(f16tof32(ubo_load_5 & 0xFFFF)), float16_t(f16tof32(ubo_load_5 >> 16))).yx;
  p[0][1] = float16_t(f16tof32(((u[0].y) & 0xFFFF)));
  return;
}
FXC validation failure:
D:\Projects\RampUp\dawn\test\tint\buffer\Shader@0x000001D1582D92D0(4,15-23): error X3000: syntax error: unexpected token 'float16_t'

