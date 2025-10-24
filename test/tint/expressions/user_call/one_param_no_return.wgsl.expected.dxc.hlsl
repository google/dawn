
void c(int z) {
  int a = asint((asuint(int(1)) + asuint(z)));
  a = asint((asuint(a) + asuint(int(2))));
}

[numthreads(1, 1, 1)]
void b() {
  c(int(2));
  c(int(3));
}

