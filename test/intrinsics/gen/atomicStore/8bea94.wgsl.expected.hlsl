groupshared int arg_0;

void atomicStore_8bea94() {
  int atomic_result = 0;
  InterlockedExchange(arg_0, 1, atomic_result);
}

[numthreads(1, 1, 1)]
void compute_main() {
  atomicStore_8bea94();
  return;
}
