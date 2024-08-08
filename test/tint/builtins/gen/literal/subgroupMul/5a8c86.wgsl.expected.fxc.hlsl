SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

int3 subgroupMul_5a8c86() {
  int3 res = WaveActiveProduct((1).xxx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(subgroupMul_5a8c86()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x000001AAF2D7BAC0(4,14-39): error X3004: undeclared identifier 'WaveActiveProduct'

