SKIP: INVALID


[numthreads(1, 1, 1)]
void f() {
  vector<float16_t, 2> a = vector<float16_t, 2>(float16_t(1.0h), float16_t(2.0h));
  vector<float16_t, 2> b = a;
}

FXC validation failure:
C:\src\dawn\Shader@0x000001E6EF884970(4,10-18): error X3000: syntax error: unexpected token 'float16_t'
C:\src\dawn\Shader@0x000001E6EF884970(5,10-18): error X3000: syntax error: unexpected token 'float16_t'

