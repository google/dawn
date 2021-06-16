[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

struct tint_array_wrapper {
  int arr[4];
};
struct S {
  tint_array_wrapper arr;
};
struct tint_array_wrapper_1 {
  tint_array_wrapper arr[2];
};

void foo() {
  const tint_array_wrapper src = {{0, 0, 0, 0}};
  tint_array_wrapper tint_symbol = {{0, 0, 0, 0}};
  S dst_struct = {{{0, 0, 0, 0}}};
  tint_array_wrapper_1 dst_array = {{{{0, 0, 0, 0}}, {{0, 0, 0, 0}}}};
  dst_struct.arr = src;
  dst_array.arr[1] = src;
  tint_symbol = src;
  dst_struct.arr = src;
  dst_array.arr[0] = src;
}
