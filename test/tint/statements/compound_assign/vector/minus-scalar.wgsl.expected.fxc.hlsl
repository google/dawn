
RWByteAddressBuffer v : register(u0);
void foo() {
  v.Store4(0u, asuint((asfloat(v.Load4(0u)) - 2.0f)));
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

