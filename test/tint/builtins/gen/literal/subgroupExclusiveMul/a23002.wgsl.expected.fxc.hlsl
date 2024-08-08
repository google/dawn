SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

int subgroupExclusiveMul_a23002() {
  int res = WavePrefixProduct(1);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupExclusiveMul_a23002()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x000001CDCCD4F3F0(4,13-32): error X3004: undeclared identifier 'WavePrefixProduct'

