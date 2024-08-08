SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

int4 subgroupExclusiveAdd_406ab4() {
  int4 res = WavePrefixSum((1).xxxx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(subgroupExclusiveAdd_406ab4()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x000002ABAC96F970(4,14-36): error X3004: undeclared identifier 'WavePrefixSum'

