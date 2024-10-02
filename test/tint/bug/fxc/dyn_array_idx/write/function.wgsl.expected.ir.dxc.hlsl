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
  int v = asint(ubo[0u].x);
  s.data[v] = int(1);
  result.Store(0u, asuint(s.data[int(3)]));
}

