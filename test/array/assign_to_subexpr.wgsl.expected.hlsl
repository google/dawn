struct S {
  int arr[4];
};

void foo() {
  const int src[4] = {0, 0, 0, 0};
  int tint_symbol[4] = {0, 0, 0, 0};
  S dst_struct = {{0, 0, 0, 0}};
  int dst_array[2][4] = {{0, 0, 0, 0}, {0, 0, 0, 0}};
  dst_struct.arr = src;
  dst_array[1] = src;
  tint_symbol = src;
  dst_struct.arr = src;
  dst_array[0] = src;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

