SKIP: INVALID


static float t = 0.0f;
float3 m() {
  t = 1.0f;
  return float3((t).xxx);
}

void f() {
  vector<float16_t, 3> v = vector<float16_t, 3>(m());
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

FXC validation failure:
C:\src\dawn\Shader@0x00000192048C1B40(9,10-18): error X3000: syntax error: unexpected token 'float16_t'

