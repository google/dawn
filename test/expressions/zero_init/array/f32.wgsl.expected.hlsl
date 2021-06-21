[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

struct tint_array_wrapper {
  float arr[4];
};

void f() {
  tint_array_wrapper v = {{0.0f, 0.0f, 0.0f, 0.0f}};
}
