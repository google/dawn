SKIP: INVALID


[numthreads(1, 1, 1)]
void f() {
  vector<float16_t, 3> a = vector<float16_t, 3>(float16_t(1.0h), float16_t(2.0h), float16_t(3.0h));
  vector<float16_t, 3> b = a;
}

FXC validation failure:
C:\src\dawn\Shader@0x0000028A0A134970(4,10-18): error X3000: syntax error: unexpected token 'float16_t'
C:\src\dawn\Shader@0x0000028A0A134970(5,10-18): error X3000: syntax error: unexpected token 'float16_t'

