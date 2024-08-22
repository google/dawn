SKIP: FAILED


RWByteAddressBuffer prevent_dce : register(u0);
int subgroupExclusiveMul_a23002() {
  int res = WavePrefixProduct(1);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(subgroupExclusiveMul_a23002()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupExclusiveMul_a23002()));
}

FXC validation failure:
C:\src\dawn\Shader@0x0000027E0520F6E0(4,13-32): error X3004: undeclared identifier 'WavePrefixProduct'

