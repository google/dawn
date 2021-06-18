groupshared int arg_0;

void atomicMin_278235() {
  int atomic_result = 0;
  InterlockedMin(arg_0, 1, atomic_result);
  int res = atomic_result;
}

[numthreads(1, 1, 1)]
void compute_main() {
  atomicMin_278235();
  return;
}
