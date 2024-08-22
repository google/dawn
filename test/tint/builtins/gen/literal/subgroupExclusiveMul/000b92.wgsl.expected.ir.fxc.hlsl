SKIP: FAILED


RWByteAddressBuffer prevent_dce : register(u0);
uint4 subgroupExclusiveMul_000b92() {
  uint4 res = WavePrefixProduct((1u).xxxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, subgroupExclusiveMul_000b92());
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, subgroupExclusiveMul_000b92());
}

FXC validation failure:
C:\src\dawn\Shader@0x0000020D0E7107F0(4,15-42): error X3004: undeclared identifier 'WavePrefixProduct'

