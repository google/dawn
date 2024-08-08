SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

uint4 subgroupAnd_d2c9a6() {
  uint4 arg_0 = (1u).xxxx;
  uint4 res = WaveActiveBitAnd(arg_0);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(subgroupAnd_d2c9a6()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x000002CB4B9D1740(5,15-37): error X3004: undeclared identifier 'WaveActiveBitAnd'

