
int c(int x, int y, int z) {
  int a = asint((asuint(asint((asuint(asint((1u + asuint(x)))) + asuint(y)))) + asuint(z)));
  a = asint((asuint(a) + 2u));
  return a;
}

[numthreads(1, 1, 1)]
void b() {
  int b_1 = c(int(2), int(3), int(4));
  int v = b_1;
  b_1 = asint((asuint(v) + asuint(c(int(3), int(4), int(5)))));
}

