groupshared uint arg_0;

void atomicAnd_34edd3() {
  uint atomic_result = 0u;
  InterlockedAnd(arg_0, 1u, atomic_result);
  uint res = atomic_result;
}

[numthreads(1, 1, 1)]
void compute_main() {
  atomicAnd_34edd3();
  return;
}
