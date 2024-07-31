SKIP: FAILED


[numthreads(1, 1, 1)]
void f() {
  vector<float16_t, 3> a = vector<float16_t, 3>(float16_t(1.0h), float16_t(2.0h), float16_t(3.0h));
  float16_t b = float16_t(4.0h);
  vector<float16_t, 3> r = (a + b);
}

FXC validation failure:
c:\src\dawn\Shader@0x00000174F877F2F0(4,10-18): error X3000: syntax error: unexpected token 'float16_t'
c:\src\dawn\Shader@0x00000174F877F2F0(5,3-11): error X3000: unrecognized identifier 'float16_t'
c:\src\dawn\Shader@0x00000174F877F2F0(5,13): error X3000: unrecognized identifier 'b'

