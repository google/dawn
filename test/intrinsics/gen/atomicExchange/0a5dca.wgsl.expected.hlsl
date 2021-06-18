groupshared uint arg_0;

void atomicExchange_0a5dca() {
  uint atomic_result = 0u;
  InterlockedExchange(arg_0, 1u, atomic_result);
  uint res = atomic_result;
}

[numthreads(1, 1, 1)]
void compute_main() {
  atomicExchange_0a5dca();
  return;
}
