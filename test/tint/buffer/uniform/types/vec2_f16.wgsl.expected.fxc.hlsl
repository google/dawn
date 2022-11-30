SKIP: FAILED

cbuffer cbuffer_u : register(b0, space0) {
  uint4 u[1];
};

[numthreads(1, 1, 1)]
void main() {
  uint ubo_load = u[0].x;
  const vector<float16_t, 2> x = vector<float16_t, 2>(float16_t(f16tof32(ubo_load & 0xFFFF)), float16_t(f16tof32(ubo_load >> 16)));
  return;
}
FXC validation failure:
D:\Projects\RampUp\dawn\test\tint\buffer\Shader@0x00000244450436F0(8,16-24): error X3000: syntax error: unexpected token 'float16_t'

