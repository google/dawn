SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

uint subgroupOr_0bc264() {
  uint res = WaveActiveBitOr(1u);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupOr_0bc264()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x00000247F8842930(4,14-32): error X3004: undeclared identifier 'WaveActiveBitOr'

