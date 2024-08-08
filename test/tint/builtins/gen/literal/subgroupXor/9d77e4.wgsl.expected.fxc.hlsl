SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

uint4 subgroupXor_9d77e4() {
  uint4 res = WaveActiveBitXor((1u).xxxx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(subgroupXor_9d77e4()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x000001B889021E80(4,15-41): error X3004: undeclared identifier 'WaveActiveBitXor'

