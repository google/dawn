SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

uint subgroupExclusiveMul_dc51f8() {
  uint arg_0 = 1u;
  uint res = WavePrefixProduct(arg_0);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupExclusiveMul_dc51f8()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x000001DD5D333B80(5,14-37): error X3004: undeclared identifier 'WavePrefixProduct'

