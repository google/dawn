SKIP: INVALID


static float t = 0.0f;
float2x3 m() {
  t = (t + 1.0f);
  return float2x3(float3(1.0f, 2.0f, 3.0f), float3(4.0f, 5.0f, 6.0f));
}

void f() {
  matrix<float16_t, 2, 3> v = matrix<float16_t, 2, 3>(m());
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

FXC validation failure:
C:\src\dawn\Shader@0x000001DD66809130(9,10-18): error X3000: syntax error: unexpected token 'float16_t'

