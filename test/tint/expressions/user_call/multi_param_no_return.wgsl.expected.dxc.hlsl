
void c(int x, int y, int z) {
  int a = asint((asuint(asint((asuint(asint((asuint(int(1)) + asuint(x)))) + asuint(y)))) + asuint(z)));
  a = asint((asuint(a) + asuint(int(2))));
}

void b() {
  c(int(1), int(2), int(3));
  c(int(4), int(5), int(6));
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

