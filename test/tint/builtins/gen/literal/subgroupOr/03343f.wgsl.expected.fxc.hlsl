SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

int3 subgroupOr_03343f() {
  int3 tint_tmp = (1).xxx;
  int3 res = asint(WaveActiveBitOr(asuint(tint_tmp)));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(subgroupOr_03343f()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x0000016D913D2270(5,20-52): error X3004: undeclared identifier 'WaveActiveBitOr'

