groupshared int arg_0;

void atomicExchange_e114ba() {
  int atomic_result = 0;
  InterlockedExchange(arg_0, 1, atomic_result);
  int res = atomic_result;
}

[numthreads(1, 1, 1)]
void compute_main() {
  atomicExchange_e114ba();
  return;
}
