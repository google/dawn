
RWByteAddressBuffer prevent_dce : register(u0);
RWByteAddressBuffer sb_rw : register(u1);
uint atomicSub_15bfc9() {
  uint arg_1 = 1u;
  uint v = 0u;
  uint v_1 = (0u - arg_1);
  sb_rw.InterlockedAdd(uint(0u), v_1, v);
  uint res = v;
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, atomicSub_15bfc9());
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, atomicSub_15bfc9());
}

