struct tint_array_wrapper {
  int arr[3];
};

[numthreads(1, 1, 1)]
void main() {
  tint_array_wrapper v = (tint_array_wrapper)0;
  v;
  return;
}
