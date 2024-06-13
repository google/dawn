uint4 subgroupBallot_7e6d0e() {
  uint4 res = WaveActiveBallot(true);
  return res;
}

RWByteAddressBuffer prevent_dce : register(u0);

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(subgroupBallot_7e6d0e()));
  return;
}
