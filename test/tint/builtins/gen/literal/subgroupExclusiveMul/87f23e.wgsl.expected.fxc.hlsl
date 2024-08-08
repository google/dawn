SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

int3 subgroupExclusiveMul_87f23e() {
  int3 res = WavePrefixProduct((1).xxx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(subgroupExclusiveMul_87f23e()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x0000019B8B1D3DD0(4,14-39): error X3004: undeclared identifier 'WavePrefixProduct'

