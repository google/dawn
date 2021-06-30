struct tint_array_wrapper {
  int arr[3];
};

[numthreads(1, 1, 1)]
void main() {
  tint_array_wrapper v = {{0, 0, 0}};
  v;
  return;
}
