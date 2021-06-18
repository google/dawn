RWByteAddressBuffer sb_rw : register(u0, space0);

void atomicExchange_f2e22f() {
  int atomic_result = 0;
  sb_rw.InterlockedExchange(0u, 0u, atomic_result);
  int res = atomic_result;
}

void fragment_main() {
  atomicExchange_f2e22f();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  atomicExchange_f2e22f();
  return;
}
