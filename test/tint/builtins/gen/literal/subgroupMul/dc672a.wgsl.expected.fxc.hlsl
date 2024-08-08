SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

uint2 subgroupMul_dc672a() {
  uint2 res = WaveActiveProduct((1u).xx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(subgroupMul_dc672a()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x00000174E653F3E0(4,15-40): error X3004: undeclared identifier 'WaveActiveProduct'

