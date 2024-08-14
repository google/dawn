SKIP: FAILED


RWByteAddressBuffer prevent_dce : register(u0);
uint3 subgroupExclusiveAdd_0ff95a() {
  uint3 res = WavePrefixSum((1u).xxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, subgroupExclusiveAdd_0ff95a());
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, subgroupExclusiveAdd_0ff95a());
}

FXC validation failure:
C:\src\dawn\Shader@0x000002565E4293A0(4,15-37): error X3004: undeclared identifier 'WavePrefixSum'

