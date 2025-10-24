
int c() {
  int a = int(1);
  a = asint((asuint(a) + asuint(int(2))));
  return a;
}

[numthreads(1, 1, 1)]
void b() {
  int b_1 = c();
  int v = c();
  b_1 = asint((asuint(b_1) + asuint(v)));
}

