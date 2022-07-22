[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

void a() {
}

void _a() {
}

void b() {
  a();
}

void _b() {
  _a();
}
