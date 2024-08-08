SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

uint subgroupXor_7750d6() {
  uint res = WaveActiveBitXor(1u);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupXor_7750d6()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x0000017025E7FFC0(4,14-33): error X3004: undeclared identifier 'WaveActiveBitXor'

