SKIP: FAILED


RWByteAddressBuffer prevent_dce : register(u0);
int3 subgroupExclusiveMul_87f23e() {
  int3 arg_0 = (1).xxx;
  int3 res = WavePrefixProduct(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(subgroupExclusiveMul_87f23e()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(subgroupExclusiveMul_87f23e()));
}

FXC validation failure:
C:\src\dawn\Shader@0x0000022F4AE6D980(5,14-37): error X3004: undeclared identifier 'WavePrefixProduct'

