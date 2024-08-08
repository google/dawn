SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

uint4 subgroupAnd_d2c9a6() {
  uint4 res = WaveActiveBitAnd((1u).xxxx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(subgroupAnd_d2c9a6()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x000001576ECA5300(4,15-41): error X3004: undeclared identifier 'WaveActiveBitAnd'

