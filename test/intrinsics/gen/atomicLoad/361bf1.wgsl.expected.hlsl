groupshared uint arg_0;

void atomicLoad_361bf1() {
  uint atomic_result = 0u;
  InterlockedOr(arg_0, 0, atomic_result);
  uint res = atomic_result;
}

[numthreads(1, 1, 1)]
void compute_main() {
  atomicLoad_361bf1();
  return;
}
