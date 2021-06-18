groupshared uint arg_0;

void atomicStore_726882() {
  uint atomic_result = 0u;
  InterlockedExchange(arg_0, 1u, atomic_result);
}

[numthreads(1, 1, 1)]
void compute_main() {
  atomicStore_726882();
  return;
}
