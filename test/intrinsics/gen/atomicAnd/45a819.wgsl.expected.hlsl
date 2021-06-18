groupshared int arg_0;

void atomicAnd_45a819() {
  int atomic_result = 0;
  InterlockedAnd(arg_0, 1, atomic_result);
  int res = atomic_result;
}

[numthreads(1, 1, 1)]
void compute_main() {
  atomicAnd_45a819();
  return;
}
