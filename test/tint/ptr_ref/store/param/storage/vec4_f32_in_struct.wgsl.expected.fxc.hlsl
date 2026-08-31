
RWByteAddressBuffer S : register(u0);
void func() {
  S.Store4(0u, (0u).xxxx);
}

[numthreads(1, 1, 1)]
void main() {
  func();
}

