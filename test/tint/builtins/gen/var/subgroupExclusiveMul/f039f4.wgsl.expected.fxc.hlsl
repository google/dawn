SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

uint3 subgroupExclusiveMul_f039f4() {
  uint3 arg_0 = (1u).xxx;
  uint3 res = WavePrefixProduct(arg_0);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(subgroupExclusiveMul_f039f4()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x0000026D3A9EEFF0(5,15-38): error X3004: undeclared identifier 'WavePrefixProduct'

