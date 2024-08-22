SKIP: FAILED

[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

void f() {
  vector<float16_t, 2> v2 = (float16_t(3.0h)).xx;
  vector<float16_t, 3> v3 = (float16_t(3.0h)).xxx;
  vector<float16_t, 4> v4 = (float16_t(3.0h)).xxxx;
}
FXC validation failure:
C:\src\dawn\Shader@0x00000252D2573B10(7,10-18): error X3000: syntax error: unexpected token 'float16_t'
C:\src\dawn\Shader@0x00000252D2573B10(8,10-18): error X3000: syntax error: unexpected token 'float16_t'
C:\src\dawn\Shader@0x00000252D2573B10(9,10-18): error X3000: syntax error: unexpected token 'float16_t'

