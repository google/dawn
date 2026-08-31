
RWByteAddressBuffer U : register(u0);
[numthreads(1, 1, 1)]
void f() {
  U.Store3(0u, uint3(1065353216u, 1073741824u, 1077936128u));
  U.Store(0u, 1065353216u);
  U.Store(4u, 1073741824u);
  U.Store(8u, 1077936128u);
}

