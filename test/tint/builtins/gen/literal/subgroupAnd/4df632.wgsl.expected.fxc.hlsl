SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

uint subgroupAnd_4df632() {
  uint res = WaveActiveBitAnd(1u);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupAnd_4df632()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x000001C62A3C1D70(4,14-33): error X3004: undeclared identifier 'WaveActiveBitAnd'

