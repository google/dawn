struct _a {
  int _b;
};


RWByteAddressBuffer s : register(u0);
[numthreads(1, 1, 1)]
void f() {
  _a v = (_a)0;
  _a c = v;
  int d = c._b;
  _a v_1 = v;
  s.Store(0u, asuint((v_1._b + d)));
}

