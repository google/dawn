[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

struct tint_array_wrapper {
  float arr[4];
};
struct S {
  tint_array_wrapper a;
};

void f() {
  S v = {{{0.0f, 0.0f, 0.0f, 0.0f}}};
}
