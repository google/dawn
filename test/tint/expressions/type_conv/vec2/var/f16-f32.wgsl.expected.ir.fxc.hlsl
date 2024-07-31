SKIP: FAILED


static vector<float16_t, 2> u = (float16_t(1.0h)).xx;
void f() {
  float2 v = float2(u);
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

FXC validation failure:
c:\src\dawn\Shader@0x00000221B10C5980(2,15-23): error X3000: syntax error: unexpected token 'float16_t'

