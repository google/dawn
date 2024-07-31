struct _A {
  int _B;
};


RWByteAddressBuffer s : register(u0);
[numthreads(1, 1, 1)]
void f() {
  _A v = (_A)0;
  _A c = v;
  int d = c._B;
  _A v_1 = v;
  s.Store(0u, asuint((v_1._B + d)));
}

