SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

uint3 subgroupMul_fa781b() {
  uint3 arg_0 = (1u).xxx;
  uint3 res = WaveActiveProduct(arg_0);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(subgroupMul_fa781b()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x00000194394D0A40(5,15-38): error X3004: undeclared identifier 'WaveActiveProduct'

