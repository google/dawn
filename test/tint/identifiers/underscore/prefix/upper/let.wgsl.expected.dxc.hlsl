
RWByteAddressBuffer s : register(u0);
[numthreads(1, 1, 1)]
void f() {
  int A = int(1);
  int _A = int(2);
  int B = A;
  int _B = _A;
  s.Store(0u, asuint((((A + _A) + B) + _B)));
}

