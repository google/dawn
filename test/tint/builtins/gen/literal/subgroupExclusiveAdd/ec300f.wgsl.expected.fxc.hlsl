SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

uint4 subgroupExclusiveAdd_ec300f() {
  uint4 res = WavePrefixSum((1u).xxxx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(subgroupExclusiveAdd_ec300f()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x0000018FC519F3F0(4,15-38): error X3004: undeclared identifier 'WavePrefixSum'

