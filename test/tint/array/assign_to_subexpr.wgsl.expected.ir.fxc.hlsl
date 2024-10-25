struct S {
  int arr[4];
};


RWByteAddressBuffer s : register(u0);
int foo() {
  int src[4] = (int[4])0;
  int tint_symbol[4] = (int[4])0;
  S dst_struct = (S)0;
  int dst_array[2][4] = (int[2][4])0;
  dst_struct.arr = src;
  dst_array[int(1)] = src;
  tint_symbol = src;
  dst_struct.arr = src;
  dst_array[int(0)] = src;
  return ((tint_symbol[int(0)] + dst_struct.arr[int(0)]) + dst_array[int(0)][int(0)]);
}

[numthreads(1, 1, 1)]
void main() {
  s.Store(0u, asuint(foo()));
}

