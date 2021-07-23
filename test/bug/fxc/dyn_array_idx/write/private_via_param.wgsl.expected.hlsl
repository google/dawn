SKIP: FAILED

cbuffer cbuffer_ubo : register(b0, space0) {
  uint4 ubo[1];
};

struct S {
  int data[64];
};

RWByteAddressBuffer result : register(u1, space0);
static S s = (S)0;

void x(inout S p) {
  p.data[asint(ubo[0].x)] = 1;
}

[numthreads(1, 1, 1)]
void f() {
  x(s);
  result.Store(0u, asuint(s.data[3]));
  return;
}
C:\src\tint\test\Shader@0x0000018B80081AA0(13,3-25): error X3500: array reference cannot be used as an l-value; not natively addressable

