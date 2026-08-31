
int c() {
  int a = int(1);
  a = asint((asuint(a) + 2u));
  return a;
}

[numthreads(1, 1, 1)]
void b() {
  int b_1 = c();
  int v = b_1;
  b_1 = asint((asuint(v) + asuint(c())));
}

