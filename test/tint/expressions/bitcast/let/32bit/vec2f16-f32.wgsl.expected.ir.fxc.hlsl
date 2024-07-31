SKIP: FAILED


float tint_bitcast_from_f16(vector<float16_t, 2> src) {
  uint2 r = f32tof16(float2(src));
  return asfloat(((r.x & 65535u) | ((r.y & 65535u) << 16u)));
}

[numthreads(1, 1, 1)]
void f() {
  vector<float16_t, 2> a = vector<float16_t, 2>(float16_t(1.0h), float16_t(2.0h));
  float b = tint_bitcast_from_f16(a);
}

FXC validation failure:
c:\src\dawn\Shader@0x00000298162F18B0(2,36-44): error X3000: syntax error: unexpected token 'float16_t'
c:\src\dawn\Shader@0x00000298162F18B0(3,29-31): error X3004: undeclared identifier 'src'
c:\src\dawn\Shader@0x00000298162F18B0(3,22-32): error X3014: incorrect number of arguments to numeric-type constructor

