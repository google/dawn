SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

uint4 subgroupOr_f915e3() {
  uint4 res = WaveActiveBitOr((1u).xxxx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(subgroupOr_f915e3()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x00000214A6D31D70(4,15-40): error X3004: undeclared identifier 'WaveActiveBitOr'

