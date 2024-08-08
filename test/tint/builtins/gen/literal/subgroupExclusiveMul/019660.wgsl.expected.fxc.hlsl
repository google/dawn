SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

int4 subgroupExclusiveMul_019660() {
  int4 res = WavePrefixProduct((1).xxxx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(subgroupExclusiveMul_019660()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x0000021F33F0C7D0(4,14-40): error X3004: undeclared identifier 'WavePrefixProduct'

