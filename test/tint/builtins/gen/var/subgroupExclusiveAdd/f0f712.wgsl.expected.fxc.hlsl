SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

int2 subgroupExclusiveAdd_f0f712() {
  int2 arg_0 = (1).xx;
  int2 res = WavePrefixSum(arg_0);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(subgroupExclusiveAdd_f0f712()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x000001A5F2BA0120(5,14-33): error X3004: undeclared identifier 'WavePrefixSum'

