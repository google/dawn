RWByteAddressBuffer prevent_dce : register(u0);

uint4 subgroupBallot_7e6d0e() {
  uint4 res = WaveActiveBallot(true);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(subgroupBallot_7e6d0e()));
  return;
}
