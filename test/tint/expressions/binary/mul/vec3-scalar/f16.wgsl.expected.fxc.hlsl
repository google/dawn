SKIP: FAILED

[numthreads(1, 1, 1)]
void f() {
  vector<float16_t, 3> a = vector<float16_t, 3>(float16_t(1.0h), float16_t(2.0h), float16_t(3.0h));
  float16_t b = float16_t(4.0h);
  vector<float16_t, 3> r = (a * b);
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x0000020440F5FA70(3,10-18): error X3000: syntax error: unexpected token 'float16_t'
C:\src\dawn\Shader@0x0000020440F5FA70(4,3-11): error X3000: unrecognized identifier 'float16_t'
C:\src\dawn\Shader@0x0000020440F5FA70(4,13): error X3000: unrecognized identifier 'b'

