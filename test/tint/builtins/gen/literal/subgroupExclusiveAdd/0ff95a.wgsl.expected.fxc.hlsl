SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

uint3 subgroupExclusiveAdd_0ff95a() {
  uint3 res = WavePrefixSum((1u).xxx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(subgroupExclusiveAdd_0ff95a()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x00000217AC3EC550(4,15-37): error X3004: undeclared identifier 'WavePrefixSum'

