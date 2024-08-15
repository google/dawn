SKIP: INVALID


float2 tint_bitcast_from_f16(vector<float16_t, 4> src) {
  uint4 r = f32tof16(float4(src));
  return asfloat(uint2(((r.x & 65535u) | ((r.y & 65535u) << 16u)), ((r.z & 65535u) | ((r.w & 65535u) << 16u))));
}

[numthreads(1, 1, 1)]
void f() {
  vector<float16_t, 4> a = vector<float16_t, 4>(float16_t(1.0h), float16_t(2.0h), float16_t(3.0h), float16_t(-4.0h));
  float2 b = tint_bitcast_from_f16(a);
}

FXC validation failure:
C:\src\dawn\Shader@0x000001D6B4E2DE70(2,37-45): error X3000: syntax error: unexpected token 'float16_t'
C:\src\dawn\Shader@0x000001D6B4E2DE70(3,29-31): error X3004: undeclared identifier 'src'
C:\src\dawn\Shader@0x000001D6B4E2DE70(3,22-32): error X3014: incorrect number of arguments to numeric-type constructor

