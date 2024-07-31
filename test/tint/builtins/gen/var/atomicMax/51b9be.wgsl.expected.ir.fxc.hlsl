
RWByteAddressBuffer prevent_dce : register(u0);
RWByteAddressBuffer sb_rw : register(u1);
uint atomicMax_51b9be() {
  uint arg_1 = 1u;
  uint v = arg_1;
  uint v_1 = 0u;
  sb_rw.InterlockedMax(uint(0u), v, v_1);
  uint res = v_1;
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, atomicMax_51b9be());
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, atomicMax_51b9be());
}

