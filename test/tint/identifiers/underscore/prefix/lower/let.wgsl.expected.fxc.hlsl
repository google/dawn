RWByteAddressBuffer s : register(u0);

[numthreads(1, 1, 1)]
void f() {
  const int a = 1;
  const int _a = a;
  const int b = a;
  const int _b = _a;
  s.Store(0u, asuint((((a + _a) + b) + _b)));
  return;
}
