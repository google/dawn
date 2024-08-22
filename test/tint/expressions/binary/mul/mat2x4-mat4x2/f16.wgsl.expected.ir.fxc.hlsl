SKIP: INVALID


[numthreads(1, 1, 1)]
void f() {
  matrix<float16_t, 2, 4> a = matrix<float16_t, 2, 4>(vector<float16_t, 4>(float16_t(1.0h), float16_t(2.0h), float16_t(3.0h), float16_t(4.0h)), vector<float16_t, 4>(float16_t(5.0h), float16_t(6.0h), float16_t(7.0h), float16_t(8.0h)));
  matrix<float16_t, 4, 2> b = matrix<float16_t, 4, 2>(vector<float16_t, 2>(float16_t(-1.0h), float16_t(-2.0h)), vector<float16_t, 2>(float16_t(-3.0h), float16_t(-4.0h)), vector<float16_t, 2>(float16_t(-5.0h), float16_t(-6.0h)), vector<float16_t, 2>(float16_t(-7.0h), float16_t(-8.0h)));
  matrix<float16_t, 4, 4> r = mul(b, a);
}

FXC validation failure:
C:\src\dawn\Shader@0x000002CEF4514010(4,10-18): error X3000: syntax error: unexpected token 'float16_t'
C:\src\dawn\Shader@0x000002CEF4514010(5,10-18): error X3000: syntax error: unexpected token 'float16_t'
C:\src\dawn\Shader@0x000002CEF4514010(6,10-18): error X3000: syntax error: unexpected token 'float16_t'

