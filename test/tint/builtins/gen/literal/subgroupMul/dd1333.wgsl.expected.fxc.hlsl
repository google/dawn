SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

uint4 subgroupMul_dd1333() {
  uint4 res = WaveActiveProduct((1u).xxxx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(subgroupMul_dd1333()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x000001D87F7EC440(4,15-42): error X3004: undeclared identifier 'WaveActiveProduct'

