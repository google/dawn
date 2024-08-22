SKIP: FAILED


RWByteAddressBuffer prevent_dce : register(u0);
int subgroupExclusiveAdd_b0c261() {
  int res = WavePrefixSum(1);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(subgroupExclusiveAdd_b0c261()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupExclusiveAdd_b0c261()));
}

FXC validation failure:
C:\src\dawn\Shader@0x000001EF5FDDFEA0(4,13-28): error X3004: undeclared identifier 'WavePrefixSum'

