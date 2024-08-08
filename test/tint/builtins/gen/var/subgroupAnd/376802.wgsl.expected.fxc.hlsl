SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

uint2 subgroupAnd_376802() {
  uint2 arg_0 = (1u).xx;
  uint2 res = WaveActiveBitAnd(arg_0);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(subgroupAnd_376802()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x000002C13E4828F0(5,15-37): error X3004: undeclared identifier 'WaveActiveBitAnd'

