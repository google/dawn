SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

int2 subgroupOr_3f60e0() {
  int2 tint_tmp = (1).xx;
  int2 res = asint(WaveActiveBitOr(asuint(tint_tmp)));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(subgroupOr_3f60e0()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x00000283D71F1990(5,20-52): error X3004: undeclared identifier 'WaveActiveBitOr'

