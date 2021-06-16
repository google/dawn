[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

struct tint_array_wrapper {
  int arr[2];
};

void foo() {
  tint_array_wrapper tint_symbol = {{0, 0}};
  tint_array_wrapper implict = {{0, 0}};
  implict = tint_symbol;
}
