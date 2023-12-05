RWByteAddressBuffer s : register(u0);

[numthreads(1, 1, 1)]
void f() {
  const int a = 1;
  const int a__ = a;
  const int b = a;
  const int b__ = a__;
  s.Store(0u, asuint((((a + a__) + b) + b__)));
  return;
}
