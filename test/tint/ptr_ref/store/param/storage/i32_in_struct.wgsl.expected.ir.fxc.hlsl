
RWByteAddressBuffer S : register(u0);
void func() {
  S.Store(0u, asuint(42));
}

[numthreads(1, 1, 1)]
void main() {
  func();
}

