SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

int2 subgroupXor_473de8() {
  int2 tint_tmp = (1).xx;
  int2 res = asint(WaveActiveBitXor(asuint(tint_tmp)));
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(subgroupXor_473de8()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(subgroupXor_473de8()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x00000214B9201060(5,20-53): error X3004: undeclared identifier 'WaveActiveBitXor'

