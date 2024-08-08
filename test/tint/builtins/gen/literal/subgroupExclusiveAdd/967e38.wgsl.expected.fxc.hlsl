SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

float subgroupExclusiveAdd_967e38() {
  float res = WavePrefixSum(1.0f);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupExclusiveAdd_967e38()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x00000192073E8300(4,15-33): error X3004: undeclared identifier 'WavePrefixSum'

