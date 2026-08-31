
void c() {
  int a = int(1);
  a = asint((asuint(a) + 2u));
}

[numthreads(1, 1, 1)]
void b() {
  c();
  c();
}

