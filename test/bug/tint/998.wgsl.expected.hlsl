SKIP: FAILED

cbuffer cbuffer_constants : register(b0, space1) {
  uint4 constants[1];
};

RWByteAddressBuffer result : register(u1, space1);

struct S {
  uint data[3];
};

static S s = (S)0;

[numthreads(1, 1, 1)]
void main() {
  s.data[constants[0].x] = 0u;
  return;
}
C:\src\tint\test\Shader@0x00000150124FBBE0(15,3-24): error X3500: array reference cannot be used as an l-value; not natively addressable

