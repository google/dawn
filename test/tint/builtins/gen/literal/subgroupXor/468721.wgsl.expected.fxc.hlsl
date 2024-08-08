SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

uint3 subgroupXor_468721() {
  uint3 res = WaveActiveBitXor((1u).xxx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(subgroupXor_468721()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x000001CEE4B02460(4,15-40): error X3004: undeclared identifier 'WaveActiveBitXor'

