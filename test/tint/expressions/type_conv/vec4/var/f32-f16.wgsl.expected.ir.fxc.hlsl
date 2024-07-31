SKIP: FAILED


static float4 u = (1.0f).xxxx;
void f() {
  vector<float16_t, 4> v = vector<float16_t, 4>(u);
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

FXC validation failure:
c:\src\dawn\Shader@0x0000022F0BF05980(4,10-18): error X3000: syntax error: unexpected token 'float16_t'

