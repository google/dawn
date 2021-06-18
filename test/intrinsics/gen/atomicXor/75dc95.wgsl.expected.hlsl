groupshared int arg_0;

void atomicXor_75dc95() {
  int atomic_result = 0;
  InterlockedXor(arg_0, 1, atomic_result);
  int res = atomic_result;
}

[numthreads(1, 1, 1)]
void compute_main() {
  atomicXor_75dc95();
  return;
}
