SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

uint2 subgroupAnd_376802() {
  uint2 res = WaveActiveBitAnd((1u).xx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(subgroupAnd_376802()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x000001F31EC14340(4,15-39): error X3004: undeclared identifier 'WaveActiveBitAnd'

