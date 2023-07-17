SKIP: FAILED

int tint_bitcast_from_f16(vector<float16_t, 2> src) {
  uint2 r = f32tof16(float2(src));
  return asint(uint((r.x & 0xffff) | ((r.y & 0xffff) << 16)));
}

[numthreads(1, 1, 1)]
void f() {
  const vector<float16_t, 2> a = vector<float16_t, 2>(float16_t(1.0h), float16_t(2.0h));
  const int b = tint_bitcast_from_f16(a);
  return;
}
FXC validation failure:
D:\Projects\RampUp\dawn\test\tint\expressions\bitcast\Shader@0x000001DC9BD890B0(1,34-42): error X3000: syntax error: unexpected token 'float16_t'
D:\Projects\RampUp\dawn\test\tint\expressions\bitcast\Shader@0x000001DC9BD890B0(2,29-31): error X3004: undeclared identifier 'src'
D:\Projects\RampUp\dawn\test\tint\expressions\bitcast\Shader@0x000001DC9BD890B0(2,22-32): error X3014: incorrect number of arguments to numeric-type constructor

