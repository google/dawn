SKIP: FAILED


static float3x2 u = float3x2(float2(1.0f, 2.0f), float2(3.0f, 4.0f), float2(5.0f, 6.0f));
void f() {
  matrix<float16_t, 3, 2> v = matrix<float16_t, 3, 2>(u);
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

FXC validation failure:
c:\src\dawn\Shader@0x0000016A740105F0(4,10-18): error X3000: syntax error: unexpected token 'float16_t'

