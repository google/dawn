SKIP: FAILED


static matrix<float16_t, 2, 2> u = matrix<float16_t, 2, 2>(vector<float16_t, 2>(float16_t(1.0h), float16_t(2.0h)), vector<float16_t, 2>(float16_t(3.0h), float16_t(4.0h)));
void f() {
  float2x2 v = float2x2(u);
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

FXC validation failure:
C:\src\dawn\Shader@0x000001D7CD8192C0(2,15-23): error X3000: syntax error: unexpected token 'float16_t'

