SKIP: FAILED


[numthreads(1, 1, 1)]
void f() {
  vector<float16_t, 3> b = vector<float16_t, 3>(float16_t(1.0h), float16_t(2.0h), float16_t(3.0h));
}

FXC validation failure:
c:\src\dawn\Shader@0x0000015F1BB119F0(4,10-18): error X3000: syntax error: unexpected token 'float16_t'

