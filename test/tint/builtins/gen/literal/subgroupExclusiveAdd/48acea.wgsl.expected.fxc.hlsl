SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

uint2 subgroupExclusiveAdd_48acea() {
  uint2 res = WavePrefixSum((1u).xx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(subgroupExclusiveAdd_48acea()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x000002434C54CE20(4,15-36): error X3004: undeclared identifier 'WavePrefixSum'

