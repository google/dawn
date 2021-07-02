[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

struct tint_array_wrapper {
  float arr[4];
};

void f() {
  tint_array_wrapper v = {(float[4])0};
}
