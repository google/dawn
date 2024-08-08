SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

int subgroupExclusiveAdd_b0c261() {
  int arg_0 = 1;
  int res = WavePrefixSum(arg_0);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupExclusiveAdd_b0c261()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x0000015B056AC640(5,13-32): error X3004: undeclared identifier 'WavePrefixSum'

