SKIP: FAILED


RWByteAddressBuffer prevent_dce : register(u0);
int subgroupMul_3fe886() {
  int res = WaveActiveProduct(1);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(subgroupMul_3fe886()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupMul_3fe886()));
}

FXC validation failure:
C:\src\dawn\Shader@0x000002C00E395EF0(4,13-32): error X3004: undeclared identifier 'WaveActiveProduct'

