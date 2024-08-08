SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

uint2 subgroupXor_7f6672() {
  uint2 res = WaveActiveBitXor((1u).xx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(subgroupXor_7f6672()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x000001D2005F1400(4,15-39): error X3004: undeclared identifier 'WaveActiveBitXor'

