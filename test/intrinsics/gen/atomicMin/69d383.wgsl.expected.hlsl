groupshared uint arg_0;

void atomicMin_69d383() {
  uint atomic_result = 0u;
  InterlockedMin(arg_0, 1u, atomic_result);
  uint res = atomic_result;
}

[numthreads(1, 1, 1)]
void compute_main() {
  atomicMin_69d383();
  return;
}
