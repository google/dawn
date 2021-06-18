groupshared int arg_0;

void atomicAdd_794055() {
  int atomic_result = 0;
  InterlockedAdd(arg_0, 1, atomic_result);
  int res = atomic_result;
}

[numthreads(1, 1, 1)]
void compute_main() {
  atomicAdd_794055();
  return;
}
