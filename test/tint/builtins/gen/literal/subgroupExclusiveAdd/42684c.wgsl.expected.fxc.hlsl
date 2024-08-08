SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

uint subgroupExclusiveAdd_42684c() {
  uint res = WavePrefixSum(1u);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupExclusiveAdd_42684c()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x000001F78FBAF3F0(4,14-30): error X3004: undeclared identifier 'WavePrefixSum'

