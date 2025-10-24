
RWByteAddressBuffer v : register(u0);
[numthreads(1, 1, 1)]
void foo() {
  v.Store4(0u, asuint((asint(v.Load4(0u)) >> ((2u).xxxx & (31u).xxxx))));
}

