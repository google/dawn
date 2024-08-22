SKIP: INVALID


void f() {
  vector<float16_t, 2> v2 = (float16_t(1.0h)).xx;
  vector<float16_t, 3> v3 = (float16_t(1.0h)).xxx;
  vector<float16_t, 4> v4 = (float16_t(1.0h)).xxxx;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

FXC validation failure:
C:\src\dawn\Shader@0x000002ED0D320FC0(3,10-18): error X3000: syntax error: unexpected token 'float16_t'
C:\src\dawn\Shader@0x000002ED0D320FC0(4,10-18): error X3000: syntax error: unexpected token 'float16_t'
C:\src\dawn\Shader@0x000002ED0D320FC0(5,10-18): error X3000: syntax error: unexpected token 'float16_t'

