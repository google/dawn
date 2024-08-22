SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

uint4 subgroupBallot_1a8251() {
  uint4 res = WaveActiveBallot(true);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(subgroupBallot_1a8251()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(subgroupBallot_1a8251()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x000002652FE860D0(4,15-36): error X3004: undeclared identifier 'WaveActiveBallot'

