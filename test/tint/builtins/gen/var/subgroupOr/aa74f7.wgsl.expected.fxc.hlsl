SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

uint2 subgroupOr_aa74f7() {
  uint2 arg_0 = (1u).xx;
  uint2 res = WaveActiveBitOr(arg_0);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(subgroupOr_aa74f7()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x00000245F48D0340(5,15-36): error X3004: undeclared identifier 'WaveActiveBitOr'

