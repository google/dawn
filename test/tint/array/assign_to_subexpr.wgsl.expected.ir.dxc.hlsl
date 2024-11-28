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
  dst_array[1u] = src;
  tint_symbol = src;
  dst_struct.arr = src;
  dst_array[0u] = src;
  return ((tint_symbol[0u] + dst_struct.arr[0u]) + dst_array[0u][0u]);
}

[numthreads(1, 1, 1)]
void main() {
  s.Store(0u, asuint(foo()));
}

