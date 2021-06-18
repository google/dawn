groupshared int arg_0;

void atomicOr_d09248() {
  int atomic_result = 0;
  InterlockedOr(arg_0, 1, atomic_result);
  int res = atomic_result;
}

[numthreads(1, 1, 1)]
void compute_main() {
  atomicOr_d09248();
  return;
}
