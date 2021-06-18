groupshared int arg_0;

void atomicMax_a89cc3() {
  int atomic_result = 0;
  InterlockedMax(arg_0, 1, atomic_result);
  int res = atomic_result;
}

[numthreads(1, 1, 1)]
void compute_main() {
  atomicMax_a89cc3();
  return;
}
