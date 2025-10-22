
int c(int x, int y, int z) {
  int a = asint((asuint(asint((asuint(asint((asuint(int(1)) + asuint(x)))) + asuint(y)))) + asuint(z)));
  a = asint((asuint(a) + asuint(int(2))));
  return a;
}

void b() {
  int b_1 = c(int(2), int(3), int(4));
  int v = c(int(3), int(4), int(5));
  b_1 = asint((asuint(b_1) + asuint(v)));
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

