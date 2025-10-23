
RWByteAddressBuffer U : register(u0);
[numthreads(1, 1, 1)]
void f() {
  U.Store3(0u, asuint(float3(1.0f, 2.0f, 3.0f)));
  U.Store(0u, asuint(1.0f));
  U.Store(4u, asuint(2.0f));
  U.Store(8u, asuint(3.0f));
}

