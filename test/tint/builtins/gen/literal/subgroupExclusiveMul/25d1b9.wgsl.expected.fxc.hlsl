SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

float2 subgroupExclusiveMul_25d1b9() {
  float2 res = WavePrefixProduct((1.0f).xx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(subgroupExclusiveMul_25d1b9()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x000001B2E00620C0(4,16-43): error X3004: undeclared identifier 'WavePrefixProduct'

