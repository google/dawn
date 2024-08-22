SKIP: INVALID


static float t = 0.0f;
float2x2 m() {
  t = (t + 1.0f);
  return float2x2(float2(1.0f, 2.0f), float2(3.0f, 4.0f));
}

void f() {
  matrix<float16_t, 2, 2> v = matrix<float16_t, 2, 2>(m());
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

FXC validation failure:
C:\src\dawn\Shader@0x0000020D920D3A60(9,10-18): error X3000: syntax error: unexpected token 'float16_t'

