struct S {
  int arr[4];
};


RWByteAddressBuffer s : register(u0);
int foo() {
  int src[4] = (int[4])0;
  int v[4] = (int[4])0;
  S dst_struct = (S)0;
  int dst_array[2][4] = (int[2][4])0;
  dst_struct.arr = src;
  dst_array[int(1)] = src;
  v = src;
  dst_struct.arr = src;
  dst_array[int(0)] = src;
  return asint((asuint(asint((asuint(v[int(0)]) + asuint(dst_struct.arr[int(0)])))) + asuint(dst_array[int(0)][int(0)])));
}

[numthreads(1, 1, 1)]
void main() {
  s.Store(0u, asuint(foo()));
}

