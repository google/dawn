
void c(int x, int y, int z) {
  int a = asint((asuint(asint((asuint(asint((1u + asuint(x)))) + asuint(y)))) + asuint(z)));
  a = asint((asuint(a) + 2u));
}

[numthreads(1, 1, 1)]
void b() {
  c(int(1), int(2), int(3));
  c(int(4), int(5), int(6));
}

