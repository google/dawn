SKIP: FAILED

vector<float16_t, 2> tint_bitcast_to_f16(float src) {
  uint v = asuint(src);
  float t_low = f16tof32(v & 0xffff);
  float t_high = f16tof32((v >> 16) & 0xffff);
  return vector<float16_t, 2>(t_low.x, t_high.x);
}

[numthreads(1, 1, 1)]
void f() {
  const float a = 2.003662109375f;
  const vector<float16_t, 2> b = tint_bitcast_to_f16(a);
  return;
}
FXC validation failure:
D:\Projects\RampUp\dawn\test\tint\expressions\bitcast\Shader@0x000002C96174CDD0(1,8-16): error X3000: syntax error: unexpected token 'float16_t'

