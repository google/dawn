SKIP: FAILED


static float3x3 u = float3x3(float3(1.0f, 2.0f, 3.0f), float3(4.0f, 5.0f, 6.0f), float3(7.0f, 8.0f, 9.0f));
void f() {
  matrix<float16_t, 3, 3> v = matrix<float16_t, 3, 3>(u);
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

FXC validation failure:
C:\src\dawn\Shader@0x000001B10D75E6D0(4,10-18): error X3000: syntax error: unexpected token 'float16_t'

