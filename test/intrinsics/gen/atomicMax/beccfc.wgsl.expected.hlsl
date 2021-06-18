groupshared uint arg_0;

void atomicMax_beccfc() {
  uint atomic_result = 0u;
  InterlockedMax(arg_0, 1u, atomic_result);
  uint res = atomic_result;
}

[numthreads(1, 1, 1)]
void compute_main() {
  atomicMax_beccfc();
  return;
}
