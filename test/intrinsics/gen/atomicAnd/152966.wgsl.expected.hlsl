RWByteAddressBuffer sb_rw : register(u0, space0);

void atomicAnd_152966() {
  int atomic_result = 0;
  sb_rw.InterlockedAnd(0u, 1, atomic_result);
  int res = atomic_result;
}

void fragment_main() {
  atomicAnd_152966();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  atomicAnd_152966();
  return;
}
