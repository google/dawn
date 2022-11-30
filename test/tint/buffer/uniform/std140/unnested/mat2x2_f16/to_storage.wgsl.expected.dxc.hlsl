cbuffer cbuffer_u : register(b0, space0) {
  uint4 u[1];
};
RWByteAddressBuffer s : register(u1, space0);

void tint_symbol(RWByteAddressBuffer buffer, uint offset, matrix<float16_t, 2, 2> value) {
  buffer.Store<vector<float16_t, 2> >((offset + 0u), value[0u]);
  buffer.Store<vector<float16_t, 2> >((offset + 4u), value[1u]);
}

matrix<float16_t, 2, 2> tint_symbol_2(uint4 buffer[1], uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  uint ubo_load = buffer[scalar_offset / 4][scalar_offset % 4];
  const uint scalar_offset_1 = ((offset + 4u)) / 4;
  uint ubo_load_1 = buffer[scalar_offset_1 / 4][scalar_offset_1 % 4];
  return matrix<float16_t, 2, 2>(vector<float16_t, 2>(float16_t(f16tof32(ubo_load & 0xFFFF)), float16_t(f16tof32(ubo_load >> 16))), vector<float16_t, 2>(float16_t(f16tof32(ubo_load_1 & 0xFFFF)), float16_t(f16tof32(ubo_load_1 >> 16))));
}

[numthreads(1, 1, 1)]
void f() {
  tint_symbol(s, 0u, tint_symbol_2(u, 0u));
  uint ubo_load_2 = u[0].x;
  s.Store<vector<float16_t, 2> >(4u, vector<float16_t, 2>(float16_t(f16tof32(ubo_load_2 & 0xFFFF)), float16_t(f16tof32(ubo_load_2 >> 16))));
  uint ubo_load_3 = u[0].x;
  s.Store<vector<float16_t, 2> >(4u, vector<float16_t, 2>(float16_t(f16tof32(ubo_load_3 & 0xFFFF)), float16_t(f16tof32(ubo_load_3 >> 16))).yx);
  s.Store<float16_t>(2u, float16_t(f16tof32(((u[0].y) & 0xFFFF))));
  return;
}
