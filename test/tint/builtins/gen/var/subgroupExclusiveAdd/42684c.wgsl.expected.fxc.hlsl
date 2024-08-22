SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

uint subgroupExclusiveAdd_42684c() {
  uint arg_0 = 1u;
  uint res = WavePrefixSum(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(subgroupExclusiveAdd_42684c()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupExclusiveAdd_42684c()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x000001427EE24EA0(5,14-33): error X3004: undeclared identifier 'WavePrefixSum'

