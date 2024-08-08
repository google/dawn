SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

uint2 subgroupOr_aa74f7() {
  uint2 res = WaveActiveBitOr((1u).xx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(subgroupOr_aa74f7()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x000001DAEF2F3690(4,15-38): error X3004: undeclared identifier 'WaveActiveBitOr'

