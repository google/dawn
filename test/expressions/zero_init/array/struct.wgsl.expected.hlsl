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
struct tint_array_wrapper {
  S arr[4];
};

void f() {
  tint_array_wrapper v = {{{0, 0u, 0.0f, false}, {0, 0u, 0.0f, false}, {0, 0u, 0.0f, false}, {0, 0u, 0.0f, false}}};
}
