groupshared uint arg_0;

void atomicOr_5e3d61() {
  uint atomic_result = 0u;
  InterlockedOr(arg_0, 1u, atomic_result);
  uint res = atomic_result;
}

[numthreads(1, 1, 1)]
void compute_main() {
  atomicOr_5e3d61();
  return;
}
