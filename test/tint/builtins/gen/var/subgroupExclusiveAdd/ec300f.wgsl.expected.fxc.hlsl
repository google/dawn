SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

uint4 subgroupExclusiveAdd_ec300f() {
  uint4 arg_0 = (1u).xxxx;
  uint4 res = WavePrefixSum(arg_0);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(subgroupExclusiveAdd_ec300f()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x000002EF15578210(5,15-34): error X3004: undeclared identifier 'WavePrefixSum'

