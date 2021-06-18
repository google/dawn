groupshared uint arg_0;

void atomicXor_c8e6be() {
  uint atomic_result = 0u;
  InterlockedXor(arg_0, 1u, atomic_result);
  uint res = atomic_result;
}

[numthreads(1, 1, 1)]
void compute_main() {
  atomicXor_c8e6be();
  return;
}
