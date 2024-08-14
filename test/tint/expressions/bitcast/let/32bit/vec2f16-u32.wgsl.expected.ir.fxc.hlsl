SKIP: FAILED


uint tint_bitcast_from_f16(vector<float16_t, 2> src) {
  uint2 r = f32tof16(float2(src));
  return ((r.x & 65535u) | ((r.y & 65535u) << 16u));
}

[numthreads(1, 1, 1)]
void f() {
  vector<float16_t, 2> a = vector<float16_t, 2>(float16_t(1.0h), float16_t(2.0h));
  uint b = tint_bitcast_from_f16(a);
}

FXC validation failure:
C:\src\dawn\Shader@0x000001B5ABF3FE30(2,35-43): error X3000: syntax error: unexpected token 'float16_t'
C:\src\dawn\Shader@0x000001B5ABF3FE30(3,29-31): error X3004: undeclared identifier 'src'
C:\src\dawn\Shader@0x000001B5ABF3FE30(3,22-32): error X3014: incorrect number of arguments to numeric-type constructor

