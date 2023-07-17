SKIP: FAILED

int2 tint_bitcast_from_f16(vector<float16_t, 4> src) {
  uint4 r = f32tof16(float4(src));
  return asint(uint2((r.x & 0xffff) | ((r.y & 0xffff) << 16), (r.z & 0xffff) | ((r.w & 0xffff) << 16)));
}

[numthreads(1, 1, 1)]
void f() {
  const vector<float16_t, 4> a = vector<float16_t, 4>(float16_t(1.0h), float16_t(2.0h), float16_t(3.0h), float16_t(-4.0h));
  const int2 b = tint_bitcast_from_f16(a);
  return;
}
FXC validation failure:
D:\Projects\RampUp\dawn\test\tint\expressions\bitcast\Shader@0x00000239A5C0BF60(1,35-43): error X3000: syntax error: unexpected token 'float16_t'
D:\Projects\RampUp\dawn\test\tint\expressions\bitcast\Shader@0x00000239A5C0BF60(2,29-31): error X3004: undeclared identifier 'src'
D:\Projects\RampUp\dawn\test\tint\expressions\bitcast\Shader@0x00000239A5C0BF60(2,22-32): error X3014: incorrect number of arguments to numeric-type constructor

