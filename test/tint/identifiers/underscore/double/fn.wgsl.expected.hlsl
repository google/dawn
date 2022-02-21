[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

void a() {
}

void a__() {
}

void b() {
  a();
}

void b__() {
  a__();
}
