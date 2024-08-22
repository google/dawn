SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

int3 subgroupExclusiveMul_87f23e() {
  int3 res = WavePrefixProduct((1).xxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(subgroupExclusiveMul_87f23e()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(subgroupExclusiveMul_87f23e()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x000001DDF1D6FD30(4,14-39): error X3004: undeclared identifier 'WavePrefixProduct'

