SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

int4 subgroupXor_83b1f3() {
  int4 tint_tmp = (1).xxxx;
  int4 res = asint(WaveActiveBitXor(asuint(tint_tmp)));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(subgroupXor_83b1f3()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x000001562F474130(5,20-53): error X3004: undeclared identifier 'WaveActiveBitXor'

