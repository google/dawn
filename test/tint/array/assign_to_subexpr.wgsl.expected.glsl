#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
struct S {
  int arr[4];
};

void foo() {
  int src[4] = int[4](0, 0, 0, 0);
  int dst[4] = int[4](0, 0, 0, 0);
  S dst_struct = S(int[4](0, 0, 0, 0));
  int dst_array[2][4] = int[2][4](int[4](0, 0, 0, 0), int[4](0, 0, 0, 0));
  dst_struct.arr = src;
  dst_array[1] = src;
  dst = src;
  dst_struct.arr = src;
  dst_array[0] = src;
}

