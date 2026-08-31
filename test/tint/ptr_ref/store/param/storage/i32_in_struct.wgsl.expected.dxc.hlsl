
RWByteAddressBuffer S : register(u0);
void func() {
  S.Store(0u, 42u);
}

[numthreads(1, 1, 1)]
void main() {
  func();
}

