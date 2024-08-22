SKIP: FAILED


RWByteAddressBuffer prevent_dce : register(u0);
uint4 subgroupAnd_d2c9a6() {
  uint4 res = WaveActiveBitAnd((1u).xxxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, subgroupAnd_d2c9a6());
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, subgroupAnd_d2c9a6());
}

FXC validation failure:
C:\src\dawn\Shader@0x00000217C80EC2E0(4,15-41): error X3004: undeclared identifier 'WaveActiveBitAnd'

