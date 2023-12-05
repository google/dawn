RWByteAddressBuffer s : register(u0);

[numthreads(1, 1, 1)]
void f() {
  const int A = 1;
  const int _A = 2;
  const int B = A;
  const int _B = _A;
  s.Store(0u, asuint((((A + _A) + B) + _B)));
  return;
}
