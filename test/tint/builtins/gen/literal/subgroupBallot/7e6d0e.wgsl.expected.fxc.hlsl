SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0, space2);

void subgroupBallot_7e6d0e() {
  uint4 res = WaveActiveBallot(true);
  prevent_dce.Store4(0u, asuint(res));
}

[numthreads(1, 1, 1)]
void compute_main() {
  subgroupBallot_7e6d0e();
  return;
}
