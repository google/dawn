
RWByteAddressBuffer v : register(u0);
[numthreads(1, 1, 1)]
void foo() {
  uint4 v_1 = asuint(asint(v.Load4(0u)));
  v.Store4(0u, asuint(asint((v_1 * asuint(int4((int(2)).xxxx))))));
}

