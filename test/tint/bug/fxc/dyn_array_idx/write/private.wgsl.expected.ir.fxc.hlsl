struct S {
  int data[64];
};


cbuffer cbuffer_ubo : register(b0) {
  uint4 ubo[1];
};
RWByteAddressBuffer result : register(u1);
static S s = (S)0;
[numthreads(1, 1, 1)]
void f() {
  int v = asint(ubo[0u].x);
  int tint_array_copy[64] = s.data;
  tint_array_copy[v] = int(1);
  int v_1[64] = tint_array_copy;
  s.data = v_1;
  result.Store(0u, asuint(s.data[int(3)]));
}

