SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

int3 subgroupExclusiveAdd_c08160() {
  int3 res = WavePrefixSum((1).xxx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(subgroupExclusiveAdd_c08160()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x00000278EC974970(4,14-35): error X3004: undeclared identifier 'WavePrefixSum'

