groupshared int arg_0;

void atomicLoad_afcc03() {
  int atomic_result = 0;
  InterlockedOr(arg_0, 0, atomic_result);
  int res = atomic_result;
}

[numthreads(1, 1, 1)]
void compute_main() {
  atomicLoad_afcc03();
  return;
}
