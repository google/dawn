SKIP: FAILED

struct S {
  int data[64];
};


cbuffer cbuffer_ubo : register(b0) {
  uint4 ubo[1];
};
RWByteAddressBuffer result : register(u1);
[numthreads(1, 1, 1)]
void f() {
  S s = (S)0;
  s.data[asint(ubo[0u].x)] = 1;
  result.Store(0u, asuint(s.data[3]));
}

FXC validation failure:
c:\src\dawn\Shader@0x000001CD7F5D0300(13,3-26): error X3500: array reference cannot be used as an l-value; not natively addressable

