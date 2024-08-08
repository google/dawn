SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

int subgroupOr_ae58b6() {
  int res = asint(WaveActiveBitOr(asuint(1)));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupOr_ae58b6()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x000001FF6F0986C0(4,19-44): error X3004: undeclared identifier 'WaveActiveBitOr'

