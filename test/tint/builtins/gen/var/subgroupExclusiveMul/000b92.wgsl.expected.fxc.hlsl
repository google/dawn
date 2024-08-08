SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

uint4 subgroupExclusiveMul_000b92() {
  uint4 arg_0 = (1u).xxxx;
  uint4 res = WavePrefixProduct(arg_0);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(subgroupExclusiveMul_000b92()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x000001777B86C6C0(5,15-38): error X3004: undeclared identifier 'WavePrefixProduct'

