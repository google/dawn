[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

struct tint_array_wrapper {
  int arr[4];
};

void f() {
  tint_array_wrapper v = {{0, 0, 0, 0}};
}
