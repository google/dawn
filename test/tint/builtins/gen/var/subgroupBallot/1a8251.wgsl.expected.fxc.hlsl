SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

uint4 subgroupBallot_1a8251() {
  bool arg_0 = true;
  uint4 res = WaveActiveBallot(arg_0);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(subgroupBallot_1a8251()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x00000216ACDFD010(5,15-37): error X3004: undeclared identifier 'WaveActiveBallot'

