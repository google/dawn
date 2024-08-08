SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

int3 subgroupAnd_1877b3() {
  int3 tint_tmp = (1).xxx;
  int3 res = asint(WaveActiveBitAnd(asuint(tint_tmp)));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(subgroupAnd_1877b3()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x000001E52F512120(5,20-53): error X3004: undeclared identifier 'WaveActiveBitAnd'

