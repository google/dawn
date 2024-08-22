SKIP: FAILED


RWByteAddressBuffer prevent_dce : register(u0);
float subgroupExclusiveAdd_967e38() {
  float res = WavePrefixSum(1.0f);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(subgroupExclusiveAdd_967e38()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupExclusiveAdd_967e38()));
}

FXC validation failure:
C:\src\dawn\Shader@0x000001B5B18B4680(4,15-33): error X3004: undeclared identifier 'WavePrefixSum'

