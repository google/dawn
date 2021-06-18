groupshared uint arg_0;

void atomicAdd_d5db1d() {
  uint atomic_result = 0u;
  InterlockedAdd(arg_0, 1u, atomic_result);
  uint res = atomic_result;
}

[numthreads(1, 1, 1)]
void compute_main() {
  atomicAdd_d5db1d();
  return;
}
