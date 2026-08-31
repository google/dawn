
void c(int z) {
  int a = asint((1u + asuint(z)));
  a = asint((asuint(a) + 2u));
}

[numthreads(1, 1, 1)]
void b() {
  c(int(2));
  c(int(3));
}

