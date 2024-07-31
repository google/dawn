struct a__ {
  int b__;
};


RWByteAddressBuffer s : register(u0);
[numthreads(1, 1, 1)]
void f() {
  a__ v = (a__)0;
  a__ c = v;
  int d = c.b__;
  a__ v_1 = v;
  s.Store(0u, asuint((v_1.b__ + d)));
}

