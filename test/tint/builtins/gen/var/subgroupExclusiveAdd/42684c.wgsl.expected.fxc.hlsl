SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

uint subgroupExclusiveAdd_42684c() {
  uint arg_0 = 1u;
  uint res = WavePrefixSum(arg_0);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupExclusiveAdd_42684c()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x000002530D1F3020(5,14-33): error X3004: undeclared identifier 'WavePrefixSum'

