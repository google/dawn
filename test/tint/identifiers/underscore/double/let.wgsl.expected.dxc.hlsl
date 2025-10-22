
RWByteAddressBuffer s : register(u0);
[numthreads(1, 1, 1)]
void f() {
  int a = int(1);
  int a__ = a;
  int b = a;
  int b__ = a__;
  s.Store(0u, asuint(asint((asuint(asint((asuint(asint((asuint(a) + asuint(a__)))) + asuint(b)))) + asuint(b__)))));
}

