
RWByteAddressBuffer v : register(u0);
[numthreads(1, 1, 1)]
void foo() {
  v.Store(0u, asuint(asint((asuint(asint(v.Load(0u))) + asuint(int(2))))));
}

