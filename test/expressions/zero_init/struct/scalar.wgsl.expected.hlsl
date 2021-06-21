[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

struct S {
  int i;
  uint u;
  float f;
  bool b;
};

void f() {
  S v = {0, 0u, 0.0f, false};
}
