SKIP: FAILED


static uint2 u = (1u).xx;
void f() {
  vector<float16_t, 2> v = vector<float16_t, 2>(u);
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

FXC validation failure:
c:\src\dawn\Shader@0x0000019DF37B5980(4,10-18): error X3000: syntax error: unexpected token 'float16_t'

