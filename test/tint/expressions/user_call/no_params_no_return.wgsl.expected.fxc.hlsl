
void c() {
  int a = int(1);
  a = asint((asuint(a) + asuint(int(2))));
}

[numthreads(1, 1, 1)]
void b() {
  c();
  c();
}

