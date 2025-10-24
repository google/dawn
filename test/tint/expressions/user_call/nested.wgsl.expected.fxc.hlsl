
void d() {
}

void c() {
  d();
}

void b() {
  c();
}

[numthreads(1, 1, 1)]
void a() {
  b();
}

