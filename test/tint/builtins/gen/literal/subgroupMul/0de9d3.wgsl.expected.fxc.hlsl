SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

float subgroupMul_0de9d3() {
  float res = WaveActiveProduct(1.0f);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupMul_0de9d3()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x000001BA1BC386C0(4,15-37): error X3004: undeclared identifier 'WaveActiveProduct'

