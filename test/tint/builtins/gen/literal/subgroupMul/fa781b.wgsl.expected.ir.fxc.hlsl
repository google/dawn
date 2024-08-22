SKIP: FAILED


RWByteAddressBuffer prevent_dce : register(u0);
uint3 subgroupMul_fa781b() {
  uint3 res = WaveActiveProduct((1u).xxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, subgroupMul_fa781b());
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, subgroupMul_fa781b());
}

FXC validation failure:
C:\src\dawn\Shader@0x0000025CA06546A0(4,15-41): error X3004: undeclared identifier 'WaveActiveProduct'

