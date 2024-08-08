SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

int2 subgroupExclusiveAdd_f0f712() {
  int2 res = WavePrefixSum((1).xx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(subgroupExclusiveAdd_f0f712()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x0000025AAAA51BF0(4,14-34): error X3004: undeclared identifier 'WavePrefixSum'

