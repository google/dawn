SKIP: INVALID


[numthreads(1, 1, 1)]
void f() {
  vector<float16_t, 3> a = vector<float16_t, 3>(float16_t(1.0h), float16_t(2.0h), float16_t(3.0h));
  vector<float16_t, 3> b = vector<float16_t, 3>(float16_t(4.0h), float16_t(5.0h), float16_t(6.0h));
  vector<float16_t, 3> r = (a * b);
}

FXC validation failure:
C:\src\dawn\Shader@0x00000227F3E29FB0(4,10-18): error X3000: syntax error: unexpected token 'float16_t'
C:\src\dawn\Shader@0x00000227F3E29FB0(5,10-18): error X3000: syntax error: unexpected token 'float16_t'
C:\src\dawn\Shader@0x00000227F3E29FB0(6,10-18): error X3000: syntax error: unexpected token 'float16_t'

