SKIP: FAILED

[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

struct S {
  vector<float16_t, 3> v;
};

static S P = (S)0;

void f() {
  P.v = vector<float16_t, 3>(float16_t(1.0h), float16_t(2.0h), float16_t(3.0h));
  P.v.x = float16_t(1.0h);
  P.v.y = float16_t(2.0h);
  P.v.z = float16_t(3.0h);
}
FXC validation failure:
D:\Projects\RampUp\dawn\test\tint\expressions\Shader@0x0000029037AE4370(7,10-18): error X3000: syntax error: unexpected token 'float16_t'

