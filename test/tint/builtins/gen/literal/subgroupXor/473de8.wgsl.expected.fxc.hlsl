SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

int2 subgroupXor_473de8() {
  int2 tint_tmp = (1).xx;
  int2 res = asint(WaveActiveBitXor(asuint(tint_tmp)));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(subgroupXor_473de8()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x000001EB105DF2E0(5,20-53): error X3004: undeclared identifier 'WaveActiveBitXor'

