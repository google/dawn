SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

uint4 subgroupExclusiveMul_000b92() {
  uint4 res = WavePrefixProduct((1u).xxxx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(subgroupExclusiveMul_000b92()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x000002078E8DF3F0(4,15-42): error X3004: undeclared identifier 'WavePrefixProduct'

